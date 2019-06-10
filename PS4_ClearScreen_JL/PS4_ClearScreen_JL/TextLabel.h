#pragma once

#include "font_memory_malloc.h"
#include "font_memory_mspace.h"
#include "Utils.h"
#include <sce_font.h>
#include <gnm.h>
#include <memory>

struct Text
{
	Vector2 Position;
	Vector2 Size;
	std::string String;
	bool Visible = true; // visible by default

	Text(Vector2 _pos, Vector2 _size, std::string _text)
	{
		Size = _size;
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
	std::shared_ptr<Text> AddText(Vector2 _pos, Vector2 _size, std::string _text);
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

