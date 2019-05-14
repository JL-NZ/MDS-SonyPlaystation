#include "TextLabel.h"
#include "Model.h"
#include "Render.h"
#include "Utils.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sceerror.h>
#include <kernel.h>
#include <mspace.h>
#include <libsysmodule.h>
#include <video_out.h>
#include <gpu_address.h>
#include <ces.h>

TextLabel::TextLabel()
{
}

TextLabel::~TextLabel()
{
}

void TextLabel::Initialize()
{
	// Get graphics context from render
	Gnm::SizeAlign textureSizeAlign = texture.initAs2d(
		1920, // width
		1080, // height
		1, // MIP level 1-16
		Gnm::kDataFormatA8Unorm, // texture format
		Gnm::kTileModeDisplay_LinearAligned, // tile mode for texture
		Gnm::kNumFragments1);

	void *textureData = Render::GetInstance()->garlicAllocator.allocate(textureSizeAlign);

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

	int surfaceBufferWidthByte = Render::GetInstance()->kDisplayBufferWidth;
	int surfaceWidthPixel = Render::GetInstance()->kDisplayBufferWidth;
	int surfaceHeightPixel = Render::GetInstance()->kDisplayBufferHeight;

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

void TextLabel::RenderFont()
{
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
}

void TextLabel::DrawText()
{
	textQuad->Draw(TextureType::RAW);
}

void TextLabel::ClearFont()
{
}
