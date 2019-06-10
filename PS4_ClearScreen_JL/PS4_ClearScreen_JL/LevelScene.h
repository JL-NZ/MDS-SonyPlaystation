#pragma once
#include "Scene.h"
#include "AudioManager.h"
#include <memory>

class LevelScene : public Scene
{
public:
	// Constructor
	LevelScene();
	// Destructor
	virtual ~LevelScene();

	//// Public Variables
	std::shared_ptr<class BallObject> ball;

	//// Public Funtions
	virtual bool Initialize(); // Should be called once, after constructor (returns false if there's an issue)
	virtual bool Update(float _deltaTick); // called every frame (returns false if there's an issue)
	virtual bool Render(); // called every frame, after Update (returns false if there's an issue)
	virtual bool RenderUI(); // called every frame, after Render (returns false if there's an issue)
	virtual bool Cleanup(); // called every frame, after everything else (returns false if there's an issue)

protected:
	//// Protected Variables
	SceScreamSoundParams m_BGMsoundParams;
	SceScreamSoundParams m_soundParams;
	SceScreamSFXBlock2* m_soundBank = nullptr;
	std::vector<std::shared_ptr<class GameObject>> m_ObjectVector;
	std::vector<std::shared_ptr<class CollectableObject>> m_CollectableVector;
	std::vector<std::shared_ptr<class Text>> m_TextVector;
	std::vector<std::shared_ptr<class SceScreamSFXBlock2>> m_SoundBanks;
	class PhysicsEngine* m_pPhysics = nullptr;
	std::shared_ptr<class Text> ScoreValueText = nullptr;
	std::shared_ptr<class Text> TimerValueText = nullptr;
	std::shared_ptr<Text> LevelCompleteText = nullptr;
	std::shared_ptr<class TextLabel> TextManager = nullptr;
	bool m_bLevelComplete = false;

	//// Protected Functions

private:
	//// Private Variables	
	


	//// Private Functions

};

