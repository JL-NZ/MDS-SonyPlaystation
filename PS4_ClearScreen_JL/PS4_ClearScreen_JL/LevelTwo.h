#pragma once
#include "LevelScene.h"
class LevelTwo :
	public LevelScene
{
public:
	// Constructor
	LevelTwo();
	// Destructor
	~LevelTwo();

	//// Public Variables

	//// Public Funtions
	bool Initialize(); // Should be called once, after constructor (returns false if there's an issue)
	bool Update(float _deltaTick); // called every frame (returns false if there's an issue)
	bool Render(); // called every frame, after Update (returns false if there's an issue)
	bool RenderUI(); // called every frame, after Render (returns false if there's an issue)	
	bool Cleanup(); // called every frame, after everything else (returns false if there's an issue)

private:
	//// Private Variables

	//// Private Functions

};