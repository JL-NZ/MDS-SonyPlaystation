#pragma once
#include "LevelScene.h"
class LevelOne :
	public LevelScene
{
public:
	// Constructor
	LevelOne();
	// Destructor
	~LevelOne();

	//// Public Variables

	//// Public Funtions
	bool Initialize(); // Should be called once, after constructor (returns false if there's an issue)
	bool Update(float _deltaTick); // called every frame (returns false if there's an issue)
	bool Render(); // called every frame, after Update (returns false if there's an issue)
	bool RenderUI(); // called every frame, after Render (returns false if there's an issue)	

private:
	//// Private Variables

	//// Private Functions

};

