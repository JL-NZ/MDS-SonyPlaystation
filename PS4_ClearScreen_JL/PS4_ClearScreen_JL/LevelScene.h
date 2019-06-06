#pragma once
#include "Scene.h"
class LevelScene : public Scene
{
public:
	// Constructor
	LevelScene();
	// Destructor
	virtual ~LevelScene();

	//// Public Variables

	//// Public Funtions
	virtual bool Initialize() = 0; // Should be called once, after constructor (returns false if there's an issue)
	virtual bool Update() = 0; // called every frame (returns false if there's an issue)
	virtual bool Render() = 0; // called every frame, after Update (returns false if there's an issue)
	virtual bool RenderUI() = 0; // called every frame, after Render (returns false if there's an issue)	

private:
	//// Private Variables

	//// Private Functions

};

