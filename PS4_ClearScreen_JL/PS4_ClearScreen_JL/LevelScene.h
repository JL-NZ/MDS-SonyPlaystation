#pragma once
#include "Scene.h"
#include <memory>

class CubeObject;

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
	virtual bool ClosingLevel(); // called before a level transition occurs

protected:
	//// Protected Variables	
	std::vector<std::shared_ptr<class GameObject>> m_ObjectVector;
	std::vector<std::shared_ptr<class CollectableObject>> m_CollectableVector;
	std::vector<std::shared_ptr<struct Text>> m_TextVector;
	std::vector<std::shared_ptr<struct SceScreamSFXBlock2>> m_SoundBanks;
	class PhysicsEngine* m_pPhysics = nullptr;
	std::shared_ptr<struct Text> ScoreValueText = nullptr;
	std::shared_ptr<struct Text> TimerValueText = nullptr;
	std::shared_ptr<struct Text> LevelCompleteText = nullptr;

	std::shared_ptr<class TextLabel> TextManager = nullptr;
	std::shared_ptr<class Light> m_pLight = nullptr;
	bool m_bLevelComplete = false;
	std::shared_ptr<CubeObject> cube = nullptr;
	unsigned int m_uiCollectableCount = 0;
	unsigned int m_uiCollectableMax = 12;
	float m_fCollectableTimer = 0.0f;
	float m_fCollectableSpawnInterval = 3.0f;
	float m_fLevelTime = 0.0f;

	//// Protected Functions
	Vector3 GetRandomPosition();
	void AddCollectable();
	void Reset();

private:
	//// Private Variables	
	bool bCheerSoundPlayed = false;


	//// Private Functions

};

