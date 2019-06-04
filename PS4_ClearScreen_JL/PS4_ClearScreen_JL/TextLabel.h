#pragma once

#include "font_memory_malloc.h"
#include "font_memory_mspace.h"
#include "Utils.h"
#include <sce_font.h>
#include <gnm.h>
#include <memory>

struct Text
{
	Vector3 Position;
	std::string String;

	Text(Vector3 _pos, std::string _text)
	{
		Position = _pos;
		String = _text;
	}
};

class TextLabel
{
public:
	TextLabel();
	~TextLabel();

	void Initialize();
	void RenderFont(std::shared_ptr<Text> _text);
	void AddText(Vector3 _pos, std::string _text);
	void DrawText();
	void ClearFont();

private:

	// Text library stuff
	SceFontRenderer s_renderer;
	SceFontHandle fontHandle;
	SceFontRenderSurface surface; // render surface struct
	SceFontHorizontalLayout HorizLayout;
	void* surfaceBuffer = NULL; // pointer to the start of the render surface
	float baseY;
	float lineH;
	float displayScaleRatio = 1.0f;

	// Initialize a Gnm::Texture object
	sce::Gnm::Texture texture;
	sce::Gnm::SizeAlign textureSizeAlign;
	// Initialize the texture sampler
	sce::Gnm::Sampler sampler;
	class Model* textQuad;
	std::vector<std::shared_ptr<Text>> textVector;

};

