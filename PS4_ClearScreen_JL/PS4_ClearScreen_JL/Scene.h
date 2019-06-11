#pragma once

// Local Includes
#include "controller.h"
#include "AudioManager.h"

// Library Includes
#include <string>
#include <memory>
#include <vector>

class Scene
{
public:
	// Constructor
	Scene();
	// Desctructor
	virtual ~Scene();

	//// Public Variables
	std::string m_Name = ""; 
	bool m_bInitialized = false;
	std::shared_ptr<ControllerContext> m_controllerContext = nullptr;
	SceScreamSoundParams m_BGMsoundParams;
	SceScreamSoundParams m_soundParams;
	SceScreamSFXBlock2* m_soundBank = nullptr;

	//// Public Functions
	virtual bool Initialize() = 0; // Should be called once, after constructor (returns false if there's an issue)
	virtual bool Update(float _deltaTick) = 0; // called every frame (returns false if there's an issue)
	virtual bool Render() = 0; // called every frame, after Update (returns false if there's an issue)
	virtual bool RenderUI() = 0; // called every frame, after Render (returns false if there's an issue)
	virtual bool Cleanup() = 0; // called every frame, after everything else (returns false if there's an issue)
	virtual bool ClosingLevel() = 0; // called before a level transition

private:
	//// Private Variables

	//// Private Functions
	



};

