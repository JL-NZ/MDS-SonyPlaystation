#include "LevelScene.h"
#include "TextLabel.h"
#include "Utils.h"
#include "GameObject.h"
#include "PhysicsEngine.h"
#include "rigidbody.h"
#include "SceneManager.h"
#include "Render.h"
#include "camera.h"

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
	Camera::GetInstance()->Initialize(90.0f, // FOV
		static_cast<float>(Render::GetInstance()->kDisplayBufferWidth) / static_cast<float>(Render::GetInstance()->kDisplayBufferHeight), // Ratio
		0.1f, // Near 
		5000.0f // Far
	);

	// Initialize physics
	m_pPhysics = PhysicsEngine::GetInstance();	

	// Text initialisation
	TextManager = std::make_shared<TextLabel>();
	TextManager->Initialize();
	std::shared_ptr<Text> ScoreText = TextManager->AddText(Vector2(0.0f, 0.0f), Vector2(50.0f), "Score:");
	std::shared_ptr<Text> TimerText = TextManager->AddText(Vector2(0.0f, 0.1f), Vector2(50.0f), "Timer:");
	ScoreValueText = TextManager->AddText(ScoreText->Position + Vector2(0.09f, 0.0f), Vector2(50.0f), "0");
	TimerValueText = TextManager->AddText(TimerText->Position + Vector2(0.09f, 0.0f), Vector2(50.0f), "0.0");

	m_TextVector.push_back(ScoreText);
	m_TextVector.push_back(TimerText);
	m_TextVector.push_back(ScoreValueText);
	m_TextVector.push_back(TimerValueText);

	// Create physics objects
	std::shared_ptr<CubeObject> cube = std::make_shared<CubeObject>(Vector3(100.0f, 0.5f, 100.0f), "/app0/normalmap.gnf");
	ball = std::make_shared<BallObject>(Vector3(1), "/app0/cat.gnf");

	// Push objects to vector
	m_ObjectVector.push_back(cube);
	m_ObjectVector.push_back(ball);

	// Set physics object positions
	cube->SetPosition(Vector3(0.0f, -1.0f, 0.0f));
	ball->SetPosition(Vector3(0.0f, 5.0f, 0.0f));

	// Sound initialization
	m_BGMsoundParams = AudioManager::GetInstance()->InitializeScreamParams();
	m_soundParams = AudioManager::GetInstance()->InitializeScreamParams();
	m_soundBank = AudioManager::GetInstance()->LoadAudioBank("/app0/SoundBank.bnk");
	m_BGMsoundParams.gain = 0.25f;

	// Play background music
	AudioManager::GetInstance()->PlaySound(m_soundBank, "bgm.wav", m_BGMsoundParams);

	m_bInitialized = true;
	
	return true;
}

bool LevelScene::Update(float _deltaTick)
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
	TimerValueText->String = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(SceneManager::GetInstance()->timeElapsed).count() / 1000);
	ScoreValueText->String = std::to_string(ball->GetScore());

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

	return true;
}
