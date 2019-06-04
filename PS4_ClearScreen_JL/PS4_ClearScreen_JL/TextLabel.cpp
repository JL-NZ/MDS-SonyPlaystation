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
	texture = Gnm::Texture();

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
		SCE_FONT_OPEN_FILE_STREAM, // load from memory
		(SceFontOpenDetail*)0,
		&fontHandle
	);

	// Set font scale and obtain layout information	
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

	baseY = HorizLayout.baseLineY;
	lineH = HorizLayout.lineHeight;
}

void TextLabel::RenderFont(std::shared_ptr<Text> _text)
{
	// Render text
	const uint8_t* utf8addr = reinterpret_cast<const uint8_t*>(_text->String.c_str());
	
	float x = (_text->Position.getX() * Render::GetInstance()->kDisplayBufferWidth) * displayScaleRatio;
	float y = (_text->Position.getY() * Render::GetInstance()->kDisplayBufferHeight) - lineH;

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

		// C0 control code processing
		if (ucode <= 0x0000001f)
		{
			if (ucode == 0x0000000a)
			{
				y += lineH;
				x = (_text->Position.getX() * Render::GetInstance()->kDisplayBufferWidth) * displayScaleRatio;
			}
			continue;
		}

		//E Render an Unicode character to the surface coordinate (x,y)
		if (y < Render::GetInstance()->kDisplayBufferHeight) {
			SceFontGlyphMetrics  metrics;
			SceFontRenderResult  result;

			ret = sceFontRenderCharGlyphImage(fontHandle, ucode, &surface, x, y, &metrics, &result);
			if (ret != SCE_FONT_OK) {
				//E Error processing
				sceFontRenderCharGlyphImage(fontHandle, '_', &surface, x, y, &metrics, &result);
			}
			//E Update the rendering position according to the information of character spacing
			x += metrics.Horizontal.advance;
		}
	}
	
}

void TextLabel::AddText(Vector3 _pos, std::string _text)
{
	std::shared_ptr<Text> TextPointer = std::make_shared<Text>(_pos, _text);
	if (TextPointer != nullptr)
	{
		textVector.push_back(TextPointer);
	}
}

void TextLabel::DrawText()
{
	this->ClearFont();
	for (int i = 0; i < textVector.size(); i++)
	{
		this->RenderFont(textVector[i]);
	}	
	textQuad->DrawSurface();
}

void TextLabel::ClearFont()
{
	int surfaceWidthPixel = Render::GetInstance()->kDisplayBufferWidth;
	int surfaceHeightPixel = Render::GetInstance()->kDisplayBufferHeight;

	// clear the buffer
	memset(surfaceBuffer, 0, surfaceWidthPixel * surfaceHeightPixel);
}
