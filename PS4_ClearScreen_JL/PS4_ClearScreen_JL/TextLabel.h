#pragma once

#include "font_memory_malloc.h"
#include "font_memory_mspace.h"
#include <sce_font.h>
#include <gnm.h>

class TextLabel
{
public:
	TextLabel();
	~TextLabel();

	void Initialize();
	void RenderFont();
	void DrawText();
	void ClearFont();

private:

	// Text library stuff
	SceFontRenderer s_renderer;
	SceFontHandle fontHandle;
	SceFontRenderSurface surface; // render surface struct
	void* surfaceBuffer = NULL; // pointer to the start of the render surface
	// Initialize a Gnm::Texture object
	sce::Gnm::Texture texture;
	sce::Gnm::SizeAlign textureSizeAlign;
	// Initialize the texture sampler
	sce::Gnm::Sampler sampler;
	class Model* textQuad;

};

