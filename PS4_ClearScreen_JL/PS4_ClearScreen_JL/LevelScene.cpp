#include "LevelScene.h"
#include "TextLabel.h"
#include "Utils.h"
#include "GameObject.h"
#include "PhysicsEngine.h"
#include "rigidbody.h"
#include "SceneManager.h"
#include "Render.h"
#include "camera.h"
#include "Light.h"
#include "Model.h"

#include <time.h>
#include <chrono>
#include <memory>

LevelScene::LevelScene()
{	
	printf("LevelScene::LevelScene \n");
}

LevelScene::~LevelScene()
{	
	printf("LevelScene::~LevelScene \n");
}

bool LevelScene::Initialize()
{
	// Enforce reset
	Reset();

	// Initialize physics
	m_pPhysics = PhysicsEngine::GetInstance();	

	// Text initialisation
	TextManager = std::make_shared<TextLabel>();
	TextManager->Initialize();

	std::shared_ptr<Text> ScoreText = TextManager->AddText(
		Vector2(0.0f, 0.0f),// position
		Vector2(50.0f), // scale
		"Score:"
	);
	std::shared_ptr<Text> TimerText = TextManager->AddText(
		Vector2(0.0f, 0.1f), // position
		Vector2(50.0f), // scale
		"Timer:"
	);

	LevelCompleteText = TextManager->AddText(
		Vector2(0.30f, 0.40f), // position
		Vector2(50.0f), // scale
		"             Level Complete \n\n\n\n\n Press X to go to the next level. \n\n\n\n\n Press Circle to go to the menu"
	);
	ScoreValueText = TextManager->AddText(
		ScoreText->Position + Vector2(0.09f, 0.0f), // position
		Vector2(50.0f), // scale
		"0"
	);
	TimerValueText = TextManager->AddText(
		TimerText->Position + Vector2(0.09f, 0.0f), // position
		Vector2(50.0f), // scale
		"0.0"
	);

	LevelCompleteText->Visible = false;

	m_TextVector.push_back(ScoreText);
	m_TextVector.push_back(TimerText);
	m_TextVector.push_back(LevelCompleteText);
	m_TextVector.push_back(ScoreValueText);
	m_TextVector.push_back(TimerValueText);

	// Create physics objects //std::shared_ptr<CubeObject> 
	cube = std::make_shared<CubeObject>(Vector3(100.0f, 0.5f, 100.0f), "/app0/normalmap.gnf");
	ball = std::make_shared<BallObject>(Vector3(1), "/app0/cat.gnf");

	// Push objects to vector
	m_ObjectVector.push_back(cube);
	m_ObjectVector.push_back(ball);

	// Set physics object positions
	cube->SetPosition(Vector3(0.0f, -1.0f, 0.0f));
	ball->SetPosition(Vector3(0.0f, 5.0f, 0.0f));

	// Sound initialization
	AudioManager::GetInstance()->Initialize();
	m_BGMsoundParams = AudioManager::GetInstance()->InitializeScreamParams();
	m_soundParams = AudioManager::GetInstance()->InitializeScreamParams();
	m_soundBank = AudioManager::GetInstance()->LoadAudioBank("/app0/SoundBank.bnk");
	m_BGMsoundParams.gain = 0.25f;

	// Create Light object
	m_pLight = std::make_shared<Light>();

	// Play background music
	AudioManager::GetInstance()->PlaySound(m_soundBank, "bgm.wav", m_BGMsoundParams);

	// Reset timer
	SceneManager::GetInstance()->timeElapsed = std::chrono::microseconds::zero();

	m_bInitialized = true;
	
	return true;
}

bool LevelScene::Update(float _deltaTick)
{
	// Check the number of collectables present in the scene
	bool iVectorEmpty = m_CollectableVector.empty();
	if (iVectorEmpty || m_fLevelTime <= 0.0f)
	{
		if (bCheerSoundPlayed == false && iVectorEmpty)
		{
			m_soundParams.gain = 0.2f;
			AudioManager::GetInstance()->PlaySound(m_soundBank, "cheer.wav", m_soundParams);
			bCheerSoundPlayed = true;
		}
		m_bLevelComplete = true;
		LevelCompleteText->Visible = true;
	}

	if (m_bLevelComplete == false)
	{
		// Check and process collisions
		for (int i = 0; i < m_ObjectVector.size(); i++)
		{
			for (int k = 0; k < m_ObjectVector.size(); k++)
			{
				// Ignore self collision
				if (k == i) continue;

				std::shared_ptr<GameObject> objectI = m_ObjectVector[i];
				std::shared_ptr<GameObject> objectK = m_ObjectVector[k];

				const sce::PhysicsEffects::PfxQueryContactInfo * contactInfo = m_pPhysics->GetCollisionInfo(
					objectI->GetRigidbody()->GetID(),
					objectK->GetRigidbody()->GetID()
				);

				// if there was contact
				if (contactInfo)
				{
					// Get user data
					void* userDataI = m_pPhysics->GetWorld()->getRigidState(contactInfo->rigidbodyIdA).getUserData();
					void* userDataK = m_pPhysics->GetWorld()->getRigidState(contactInfo->rigidbodyIdB).getUserData();

					objectI->ProcessCollision(objectK, userDataI);
					objectK->ProcessCollision(objectI, userDataK);
				}
			}
		}

		// Update physics objects
		for (int i = 0; i < m_ObjectVector.size(); i++)
		{
			if (m_ObjectVector[i])
			{
				m_ObjectVector[i]->Update(_deltaTick);
			}
		}

		// Move Camera
		sce::PhysicsEffects::PfxVector3 ballPosition = ball->GetRigidbody()->GetState().getPosition();
		Camera::GetInstance()->SetPosition(Vector3(ballPosition.getX(), ballPosition.getY(), ballPosition.getZ()) + Vector3(0.0f, 10.0f, -10.0f));
		Camera::GetInstance()->PointAt(Vector3(ballPosition.getX(), ballPosition.getY(), ballPosition.getZ()));

		// Update text stuff
		m_fLevelTime -= _deltaTick;
		TimerValueText->String = std::to_string(m_fLevelTime);//std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(SceneManager::GetInstance()->timeElapsed).count() / 1000);
		ScoreValueText->String = std::to_string(ball->GetScore());

		// Play sounds
		if (ball->m_bGrounded && ball->m_bMoving && ball->m_fSoundCD <= 0.0f)
		{
			m_soundParams.gain = 1.5f;
			AudioManager::GetInstance()->PlaySound(m_soundBank, "roll.wav", m_soundParams);
			ball->m_fSoundCD = 0.75f;
		}
	}	

	return true;
}

bool LevelScene::Render()
{
	// Render physics objects
	for (int i = 0; i < m_ObjectVector.size(); i++)
	{
		if (m_ObjectVector[i])
		{
			m_ObjectVector[i]->Render();
		}
	}

	
	return true;
}

bool LevelScene::RenderUI()
{
	// Render UI		
	TextManager->DrawText();

	return true;
}

bool LevelScene::Cleanup()
{
	// Destroy objects that are flagged for destruction
	for (int i = 0; i < m_ObjectVector.size(); i++)
	{
		if (m_ObjectVector[i]->GetDestroyFlag())
		{
			m_ObjectVector[i]->FinishDestruction();
			m_pPhysics->GetWorld()->removeRigidBody(m_ObjectVector[i]->GetRigidbody()->GetID());
			m_ObjectVector.erase(m_ObjectVector.begin() + i);
		}
	}
	
	for (int i = 0; i < m_CollectableVector.size(); i++)
	{
		if (m_CollectableVector[i]->GetDestroyFlag())
		{
			m_CollectableVector.erase(m_CollectableVector.begin() + i);
		}
	}

	return true;
}

bool LevelScene::ClosingLevel()
{
	AudioManager::GetInstance()->Uninitialize();
	return true;
}

// Obtains a random position to be used for spawning a collectable
Vector3 LevelScene::GetRandomPosition() {
	Vector3 scale = cube->GetModel()->scale;
	Vector3 position = Vector3(2.0f);
	position.setX((2.0f * Utils::GetRandomFloat() - 1.0f) * scale.getX()/2.0f);
	position.setZ((2.0f * Utils::GetRandomFloat() - 1.0f) * scale.getZ()/2.0f);
	return position;
}

// Create a new collectable
void LevelScene::AddCollectable() {
	std::shared_ptr<CollectableObject> collectable0 = std::make_shared<CollectableObject>(Vector3(1.5f), "/app0/kanna.gnf");
	m_ObjectVector.push_back(collectable0);
	m_CollectableVector.push_back(collectable0);
	collectable0->SetPosition(GetRandomPosition());
	++m_uiCollectableCount;
}

// Cleans the level
void LevelScene::Reset() {
	// Clear physics objects
	if (!m_ObjectVector.empty()) {
		// Remove rigidbodies
		for (auto& object : m_ObjectVector) {
			m_pPhysics->GetWorld()->removeRigidBody(object->GetRigidbody()->GetID());
		}
		// Clear vector
		m_ObjectVector.clear();
	}

	// Clear collectables
	m_uiCollectableCount = 0;
	if (!m_CollectableVector.empty()) {
		m_CollectableVector.clear();
	}

	bCheerSoundPlayed = false;

	// Clear level text
	m_TextVector.clear();
}