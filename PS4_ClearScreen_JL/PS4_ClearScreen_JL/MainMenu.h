#pragma once
#include "Scene.h"
class MainMenu :
	public Scene
{
public:
	// Constructor
	MainMenu();
	// Destructor
	~MainMenu();

	//// Public Variables

	//// Public Funtions
	bool Initialize(); // Should be called once, after constructor (returns false if there's an issue)
	bool Update(float _deltaTick); // called every frame (returns false if there's an issue)
	bool Render(); // called every frame, after Update (returns false if there's an issue)
	bool RenderUI(); // called every frame, after Render (returns false if there's an issue)
	bool Cleanup(); // called every frame, after everything else (returns false if there's an issue)
	bool ClosingLevel(); 

private:
	//// Private Variables
	std::vector<std::shared_ptr<class Text>> m_TextVector;
	std::shared_ptr<class TextLabel> TextManager = nullptr;

	//// Private Functions

};


