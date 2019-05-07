#include "Render.h"


#include <stdlib.h>

#include "font_memory_malloc.h"
#include "font_memory_mspace.h"
#include "GNFLoader.h"
#include "Model.h"

#include <libsysmodule.h>
#include <ces.h>

using namespace sce;
using namespace sce::Gnmx;

Render* Render::p_Render = nullptr;
size_t sceLibcHeapSize = 256 * 1024 * 1024;


Render::Render()
{	
	static const size_t kOnionMemorySize = 16 * 1024 * 1024;
	onionAllocator.initialize(
		kOnionMemorySize,
		SCE_KERNEL_WB_ONION,
		SCE_KERNEL_PROT_CPU_RW |
		SCE_KERNEL_PROT_GPU_ALL);

	static const size_t kGarlicMemorySize = 128 * 1024 * 1024;
	garlicAllocator.initialize(
		kGarlicMemorySize,
		SCE_KERNEL_WC_GARLIC,
		SCE_KERNEL_PROT_CPU_WRITE |
		SCE_KERNEL_PROT_GPU_ALL);

	// Initialize the Toolkit allocators wrapper
	onionAllocator.getIAllocator(toolkitAllocators.m_onion);
	garlicAllocator.getIAllocator(toolkitAllocators.m_garlic);

	// Initialize the Toolkit module
	Toolkit::initializeWithAllocators(&toolkitAllocators);

	// Open the video output port.
	videoOutHandle = sceVideoOutOpen(
		0, //user Id
		SCE_VIDEO_OUT_BUS_TYPE_MAIN, // bus type
		0, //index
		NULL); //param
	if (videoOutHandle < 0)
	{
		printf("sceVideoOutOpen failed: 0x%08X\n", videoOutHandle);
	}

	// Initialize the flip rate: 0: 60Hz, 1: 30Hz or 2: 20Hz.
	int ret = sceVideoOutSetFlipRate(videoOutHandle, 0);
	if (ret != SCE_OK)
	{
		printf("sceVideoOutSetFlipRate failed: 0x%08X\n", ret);
	}

	// Create the event queue for used to synchronize with end-of-pipe interrupts
	ret = sceKernelCreateEqueue(&eopEventQueue, "EOP QUEUE");
	if (ret != SCE_OK)
	{
		printf("sceKernelCreateEqueue failed: 0x%08X\n", ret);		
	}

	// Register for the end-of-pipe events.
	ret = Gnm::addEqEvent(eopEventQueue, Gnm::kEqEventGfxEop, NULL);
	if (ret != SCE_OK)
	{
		printf("Gnm::addEqEvent failed: 0x%08X\n", ret);
	}

	for (uint32_t i = 0; i < kRenderContextCount; ++i)
	{
		// Best practice: use 64 CUE ring entries for performance reasons const
		uint32_t kCueRingEntries = 64;
		renderContexts[i].cueHeap = garlicAllocator.allocate(Gnmx::ConstantUpdateEngine::computeHeapSize(kCueRingEntries), Gnm::kAlignmentOfBufferInBytes);
		const uint32_t kDcbSizeInBytes = 2 * 1024 * 1024;
		const uint32_t kCcbSizeInBytes = 256* 1024;
		renderContexts[i].dcbBuffer = onionAllocator.allocate(kDcbSizeInBytes, Gnm::kAlignmentOfBufferInBytes);
		renderContexts[i].ccbBuffer = onionAllocator.allocate(kCcbSizeInBytes, Gnm::kAlignmentOfBufferInBytes);
		
		if (!renderContexts[i].dcbBuffer)
		{
			printf("Cannot allocate the draw command buffer memory\n");
		}
		if (!renderContexts[i].ccbBuffer)
		{
			printf("Cannot allocate the constants command buffer memory\n");
		}
		if (!renderContexts[i].cueHeap)
		{
			printf("Cannot allocate the CUE heap memory\n");
		}

		// Initialize the context
		renderContexts[i].gfxContext.init(
			renderContexts[i].cueHeap, kCueRingEntries,
			renderContexts[i].dcbBuffer, kDcbSizeInBytes,
			renderContexts[i].ccbBuffer, kCcbSizeInBytes
		);
		
		renderContexts[i].eopLabel = (volatile uint*)onionAllocator.allocate(4, 8);
		renderContexts[i].contextLabel = (volatile uint*)onionAllocator.allocate(4, 8);
		if (!renderContexts[i].contextLabel)
		{
			printf("Cannot allocate a GPU label\n");
		}
		if (!renderContexts[i].eopLabel)
		{
			printf("Cannot allocate a EOP label\n");
		}
		
		renderContexts[i].eopLabel[0] = kEopStateFinished;
		renderContexts[i].contextLabel[0] = kRenderContextFree;
	}

	// Convenience array used by sceVideoOutRegisterBuffers()
	void *surfaceAddresses[kDisplayBufferCount];

	for (int i = 0; i < kDisplayBufferCount; i++)
	{
		Gnm::TileMode tileMode;
		Gnm::DataFormat format = Gnm::kDataFormatB8G8R8A8UnormSrgb;
		ret = GpuAddress::computeSurfaceTileMode(&tileMode, GpuAddress::kSurfaceTypeColorTargetDisplayable, format, 1);
		if (ret != SCE_OK)
		{
			printf("GpuAddress::computeSurfaceTileMode: 0x%08X\n", ret);
		}

		// Initialize the render target descriptor
		Gnm::RenderTargetSpec spec;
		spec.init();
		spec.m_width = kDisplayBufferWidth;
		spec.m_height = kDisplayBufferHeight;
		spec.m_pitch = 0;
		spec.m_numSlices = 1;
		spec.m_colorFormat = format;
		spec.m_colorTileModeHint = tileMode;
		spec.m_minGpuMode = gpuMode;

		spec.m_numSamples = Gnm::kNumSamples1;
		spec.m_numFragments = Gnm::kNumFragments1;
		spec.m_flags.enableCmaskFastClear = 0;
		spec.m_flags.enableFmaskCompression = 0;
		displayBuffers[i].renderTarget.init(&spec);

		const Gnm::SizeAlign sizeAlign = displayBuffers[i].renderTarget.getColorSizeAlign();		
		surfaceAddresses[i] = garlicAllocator.allocate(sizeAlign); //allocate
		if (!surfaceAddresses[i])
		{
			printf("Cannot allocate the render target memory\n");
		}

		displayBuffers[i].renderTarget.setAddresses(surfaceAddresses[i], 0, 0);
		displayBuffers[i].displayIndex = i;

	}

	// Set up the initialization parameters for the VideoOut library.
	SceVideoOutBufferAttribute videoOutBufferAttribute;
	sceVideoOutSetBufferAttribute(
		&videoOutBufferAttribute,
		SCE_VIDEO_OUT_PIXEL_FORMAT_B8_G8_R8_A8_SRGB,
		SCE_VIDEO_OUT_TILING_MODE_TILE,
		SCE_VIDEO_OUT_ASPECT_RATIO_16_9,
		backBuffer->renderTarget.getWidth(),
		backBuffer->renderTarget.getHeight(),
		backBuffer->renderTarget.getPitch());

	//Register display buffers to the slot : [kDisplayBufferCount - 1].
	ret = sceVideoOutRegisterBuffers(
		videoOutHandle,
		0, // Start index.
		surfaceAddresses, // Display buffer base addresses
		kDisplayBufferCount,
		&videoOutBufferAttribute);
	if (ret != SCE_OK)
	{
		printf("sceVideoOutRegisterBuffers failed: 0x%08X\n", ret);
	}

	Gnm::DataFormat depthFormat = Gnm::DataFormat::build(kZFormat);
	Gnm::TileMode depthTileMode;
	GpuAddress::computeSurfaceTileMode(&depthTileMode, GpuAddress::kSurfaceTypeDepthOnlyTarget, depthFormat, 1);
	
	Gnm::SizeAlign depthTargetSizeAlign = depthTarget.init(kDisplayBufferWidth, kDisplayBufferHeight, depthFormat.getZFormat(), kStencilFormat,
		depthTileMode, Gnm::kNumFragments1, kStencilFormat != Gnm::kStencilInvalid ? &stencilSizeAlign : NULL, kHtileEnabled ? &htileSizeAlign : NULL);
	
	if (kHtileEnabled) {
		htileSizeAlign = depthTarget.getHtileSizeAlign();
		void *htileMemory = garlicAllocator.allocate(htileSizeAlign);
		if (!htileMemory)
		{
			printf("Cannot allocate the HTILE buffer\n");
		}

		depthTarget.setHtileAddress(garlicAllocator.allocate(htileSizeAlign));
	}

	void *stencilMemory = NULL;
	if (kStencilFormat != Gnm::kStencilInvalid) {
		stencilMemory = garlicAllocator.allocate(stencilSizeAlign);
		if (!stencilMemory)
		{
			printf("Cannot allocate the stencil buffer\n");
		}
	}

	// Allocate the depth buffer
	void *depthMemory = garlicAllocator.allocate(depthTargetSizeAlign);
	if (!depthMemory)
	{
		printf("Cannot allocate the depth buffer\n");
	}
	depthTarget.setAddresses(garlicAllocator.allocate(depthTargetSizeAlign), stencilMemory);


	// Initialize texture stuff for the text stuff
	// Get graphics context from render
	Gnm::SizeAlign textureSizeAlign = texture.initAs2d(
		1920, // width
		1080, // height
		1, // MIP level 1-16
		Gnm::kDataFormatA8Unorm, // texture format
		Gnm::kTileModeDisplay_LinearAligned, // tile mode for texture
		Gnm::kNumFragments1);

	void *textureData = garlicAllocator.allocate(textureSizeAlign);

	if (!textureData)
	{
		printf("Cannot allocate the texture data \n");
		//return SCE_KERNEL_ERROR_ENOMEM;
	}

	texture.setBaseAddress(textureData);
	surfaceBuffer = textureData;
	textQuad = new Model(ModelType::kQuad);
	textQuad->SetTexture(texture);
	textQuad->genFetchShaderAndOffsetCache("/app0/shader_vv.sb", "/app0/TextShader_p.sb");
	

	// Initialize text library stuff
	sceSysmoduleLoadModule(SCE_SYSMODULE_FONT);
	sceSysmoduleLoadModule(SCE_SYSMODULE_FONT_FT);
	sceSysmoduleLoadModule(SCE_SYSMODULE_FREETYPE_OT);

	// Define usable memory
	static uint64_t s_fontMemoryBuffer[10 * 1024 * 1024];

	// Create memory definition
	SceFontMemory* s_fontMemory = fontMemoryCreateByMspace(&s_fontMemoryBuffer[0], sizeof(s_fontMemoryBuffer));

	// Create font library
	SceFontLibrary s_fontLib;
	sceFontCreateLibrary(s_fontMemory, sceFontSelectLibraryFt(0), &s_fontLib);

	// Let library support system fonts
	sceFontSupportSystemFonts(s_fontLib);

	// Let library support external fonts
	sceFontSupportExternalFonts(
		s_fontLib,
		16, // max number of external fonts
		SCE_FONT_FORMAT_OPENTYPE // format
	); 

	// create and prepare font renderer
	sceFontCreateRenderer(
		s_fontMemory,
		sceFontSelectRendererFt(0), //renderer type, subset is set to 0
		&s_renderer
	);

	// open font set and obtain font handle
	sceFontOpenFontSet(
		s_fontLib,
		SCE_FONT_SET_SST_STD_JAPANESE_JP_W1G, // font type
		SCE_FONT_OPEN_ON_MEMORY, // load from memory
		(SceFontOpenDetail*)0,
		&fontHandle
	);

	// Set font scale and obtain layout information
	SceFontHorizontalLayout HorizLayout;
	sceFontSetScalePixel(fontHandle, 32.f, 32.f); // set h & v scale
	sceFontGetHorizontalLayout(fontHandle, &HorizLayout);

	// Bind renderer to font
	sceFontBindRenderer(fontHandle, s_renderer);

	// Prepare rendering environment
	/*Set font scale to be applied while rendering*/
	sceFontSetupRenderScalePixel(fontHandle, 100.0f, 100.0f);

	int surfaceBufferWidthByte = kDisplayBufferWidth;
	int surfaceWidthPixel = kDisplayBufferWidth;
	int surfaceHeightPixel = kDisplayBufferHeight;

	// prepare rendering destination surface		
	sceFontRenderSurfaceInit(
		&surface, // struct to initlize
		surfaceBuffer, // address of render surface buffer
		surfaceBufferWidthByte, // width of buffer in bytes
		1, // pixel byte size
		surfaceWidthPixel, // surface width in pixels
		surfaceHeightPixel // surface height in pixels
	);

	// clear the buffer
	memset(surfaceBuffer, 0, 1 * surfaceWidthPixel * surfaceHeightPixel);
}


Render::~Render()
{

}

void Render::StartRender()
{
	while (renderContext->eopLabel[0] != kEopStateFinished) {
		// Wait for the EOP event
		SceKernelEvent eopEvent;
		int count;
		sceKernelWaitEqueue(Render::GetInstance()->eopEventQueue, &eopEvent, 1, &count, NULL);
	}

	while (renderContext->contextLabel[0] != kRenderContextFree)
	{
		// Wait for the EOP event
		SceKernelEvent eopEvent;
		int count;
		uint32_t ret = sceKernelWaitEqueue(eopEventQueue, &eopEvent, 1, &count, NULL);
		if (ret != SCE_OK)
		{
			printf("sceKernelWaitEqueue failed: 0x%08X\n", ret);
		}
	}

	Gnmx::GnmxGfxContext &gfxc = Render::GetInstance()->renderContext->gfxContext;
	renderContext->contextLabel[0] = kRenderContextInUse;
	
	// Reset the graphical context and initialize the hardware state
	gfxc.reset();
	gfxc.initializeDefaultHardwareState();
	gfxc.waitUntilSafeForRendering(videoOutHandle, backBuffer->displayIndex);

	//Transformation from clip-space to screen-space
	gfxc.setupScreenViewport(
		0, // Left
		0, // Top
		backBuffer->renderTarget.getWidth(),
		backBuffer->renderTarget.getHeight(),
		0.5f, // Z-scale
		0.5f // Z-offset
	);

	// Bind the render & depth targets to the context
	gfxc.setRenderTarget(0, &backBuffer->renderTarget);
	gfxc.setDepthRenderTarget(&depthTarget);

	Toolkit::SurfaceUtil::clearRenderTarget(gfxc, &backBuffer->renderTarget, kClearColor); //clear color buffer bit
	Toolkit::SurfaceUtil::clearDepthTarget(gfxc, &depthTarget, 1.f); //clear depth buffer bit	

	// Render text
	float x = 0.0f;
	float y = 0.0f;	

	float    displayScaleRatio = 1.0f;
	uint32_t displayWidth = 1920;
	uint32_t displayHeight = 1080;
	float baseY, lineH;
	SceFontHandle   hfont;

	const uint8_t* utf8addr;
	float x0 = 100.f * displayScaleRatio;

	SceFontRenderSurface renderSurface;

	utf8addr = reinterpret_cast<const uint8_t*>("Simple Step Sample Program\n"
		"Simple Step JAPANESE\n");
	x = x0;
	y = displayHeight * 0.5f - lineH;

	while (1) {
		uint32_t len;
		uint32_t ucode;

		//E Retrieve UTF-32 character (unicode value) one by one from UTF-8 strings
		int ret = sceCesUtf8ToUtf32(utf8addr, 4, &len, &ucode);
		if (ret != SCE_OK) {
			break;
		}
		if (ucode == 0x00000000) break;
		utf8addr += len;

		//E C0 control code processing
		if (ucode <= 0x0000001f) {
			if (ucode == 0x0000000a) {
				y += lineH;
				x = x0;
			}
			continue;
		}

		//E Render an Unicode character to the surface coordinate (x,y)
		if (y < displayHeight) {
			SceFontGlyphMetrics  metrics;
			SceFontRenderResult  result;

			ret = sceFontRenderCharGlyphImage(hfont, ucode, &renderSurface, x, y, &metrics, &result);
			if (ret != SCE_FONT_OK) {
				//E Error processing
				sceFontRenderCharGlyphImage(hfont, '_', &renderSurface, x, y, &metrics, &result);
			}
			//E Update the rendering position according to the information of character spacing
			x += metrics.Horizontal.advance;
		}
	}

	textQuad->Draw(TextureType::RAW);
}

void Render::SetPipelineState()
{
	Gnmx::GnmxGfxContext &gfxc = Render::GetInstance()->renderContext->gfxContext;


	// Enable z-writes using a less comparison function
	Gnm::DepthStencilControl dsc;
	dsc.init();
	dsc.setDepthControl(Gnm::kDepthControlZWriteEnable, Gnm::kCompareFuncLess);
	dsc.setDepthEnable(true);
	gfxc.setDepthStencilControl(dsc);

	//Cull clock-wise backfaces
	ToggleBackfaceCulling(true);

	// Setup an additive blending mode
	Gnm::BlendControl blendControl;
	blendControl.init();
	blendControl.setBlendEnable(true);
	blendControl.setColorEquation(
		Gnm::kBlendMultiplierSrcAlpha, 
		Gnm::kBlendFuncAdd,
		Gnm::kBlendMultiplierOneMinusSrcAlpha);
	gfxc.setBlendControl(0, blendControl);

	// A zero bit disables writing to that channel and a one bit	enables writing to that channel
	gfxc.setRenderTargetMask(0xF); // set to 1
}

void Render::EndRender()
{
	Gnmx::GnmxGfxContext &gfxc = Render::GetInstance()->renderContext->gfxContext;

	int32_t ret = gfxc.submitAndFlipWithEopInterrupt(
		videoOutHandle,
		backBuffer->displayIndex,
		SCE_VIDEO_OUT_FLIP_MODE_VSYNC,
		0,
		sce::Gnm::kEopFlushCbDbCaches,
		const_cast<uint32_t*>(renderContext->contextLabel),
		kRenderContextFree,
		sce::Gnm::kCacheActionWriteBackAndInvalidateL1andL2);

	if (ret != sce::Gnm::kSubmissionSuccess)
	{
		// Analyze the error code to determine whether the command buffers
		// have been submitted to the GPU or not
		if (ret & sce::Gnm::kStatusMaskError)
		{
			// Error codes in the kStatusMaskError family block submissions
			// so we need to mark this render context as not-in-flight
			renderContext->contextLabel[0] = kRenderContextFree;
		}

		printf("GfxContext::submitAndFlip failed: 0x%08X\n", ret);
	}

	// Signal the system that every draw for this frame has been submitted.
	// This function gives permission to the OS to hibernate when all the
	// currently running GPU tasks (graphics and compute) are done.
	ret = Gnm::submitDone();
	if (ret != SCE_OK)
	{
		printf("Gnm::submitDone failed: 0x%08X\n", ret);
	}

	// Rotate the display buffers
	backBufferIndex = (backBufferIndex + 1) % kDisplayBufferCount;
	backBuffer = displayBuffers + backBufferIndex;

	// Rotate the render contexts
	renderContextIndex = (renderContextIndex + 1) % kRenderContextCount;
	renderContext = renderContexts + renderContextIndex;
}

void Render::ToggleBackfaceCulling(bool _bool)
{
	Gnmx::GnmxGfxContext &gfxc = Render::GetInstance()->renderContext->gfxContext;
	Gnm::PrimitiveSetup primSetupReg;

	if (_bool)
	{		
		primSetupReg.init();
		primSetupReg.setCullFace(Gnm::kPrimitiveSetupCullFaceBack);
		primSetupReg.setFrontFace(Gnm::kPrimitiveSetupFrontFaceCw);
		gfxc.setPrimitiveSetup(primSetupReg);
	}
	else
	{
		
		primSetupReg.init();
		primSetupReg.setCullFace(Gnm::kPrimitiveSetupCullFaceNone);
		primSetupReg.setFrontFace(Gnm::kPrimitiveSetupFrontFaceCw);
		gfxc.setPrimitiveSetup(primSetupReg);
	}
}


Render * Render::GetInstance()
{
	if (!p_Render)
		p_Render = new Render();
	return p_Render;
}

void Render::Destroy()
{
	// Wait for the GPU to be idle before deallocating its resources
	for (uint32_t i = 0; i < kRenderContextCount; ++i)
	{
		if (renderContexts[i].contextLabel)
		{
			while (renderContexts[i].contextLabel[0] != kRenderContextFree)
			{
				sceKernelUsleep(1000);
			}
		}
	}

	// Unregister the EOP event queue
	uint32_t ret = Gnm::deleteEqEvent(eopEventQueue, Gnm::kEqEventGfxEop);
	if (ret != SCE_OK)
	{
		printf("Gnm::deleteEqEvent failed: 0x%08X\n", ret);
	}

	// Destroy the EOP event queue
	ret = sceKernelDeleteEqueue(eopEventQueue);
	if (ret != SCE_OK)
	{
		printf("sceKernelDeleteEqueue failed: 0x%08X\n", ret);
	}

	// Terminate the video output
	ret = sceVideoOutClose(videoOutHandle);
	if (ret != SCE_OK)
	{
		printf("sceVideoOutClose failed: 0x%08X\n", ret);
	}

	// Releasing manually each allocated resource is not necessary as we are
	// terminating the linear allocators for ONION and GARLIC here.
	onionAllocator.terminate();
	garlicAllocator.terminate();

	if (p_Render)
		delete p_Render;
	p_Render = nullptr;
}