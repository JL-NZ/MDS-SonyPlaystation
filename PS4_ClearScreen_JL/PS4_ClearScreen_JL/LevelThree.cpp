#include "LevelThree.h"
#include "TextLabel.h"
#include "Utils.h"
#include "GameObject.h"
#include "SceneManager.h"
#include "AudioManager.h"

#include <memory>


LevelThree::LevelThree()
{
}


LevelThree::~LevelThree()
{
}

bool LevelThree::Initialize()
{
	LevelScene::Initialize();

	std::shared_ptr<CollectableObject> collectable0 = std::make_shared<CollectableObject>(Vector3(1.5f), "/app0/kanna.gnf");
	std::shared_ptr<CollectableObject> collectable1 = std::make_shared<CollectableObject>(Vector3(1.5f), "/app0/kanna.gnf");
	std::shared_ptr<CollectableObject> collectable2 = std::make_shared<CollectableObject>(Vector3(1.5f), "/app0/kanna.gnf");

	// Push objects to vector	
	m_ObjectVector.push_back(collectable0);
	m_ObjectVector.push_back(collectable1);
	m_ObjectVector.push_back(collectable2);
	m_CollectableVector.push_back(collectable0);
	m_CollectableVector.push_back(collectable1);
	m_CollectableVector.push_back(collectable2);

	// Set physics object positions	
	collectable0->SetPosition(Vector3(-10.0f, 2.0f, 0.0f));
	collectable1->SetPosition(Vector3(5.0f, 2.0f, 0.0f));
	collectable2->SetPosition(Vector3(-5.0f, 2.0f, 5.0f));

	return true;
}

bool LevelThree::Update(float _deltaTick)
{
	LevelScene::Update(_deltaTick);

	if (m_bLevelComplete)
	{
		if (m_controllerContext->isButtonDown(0, BUTTON_CROSS))
		{
			// open next level
			SceneManager::GetInstance()->OpenLevel("Game Over Scene");
		}
		else if (m_controllerContext->isButtonDown(0, BUTTON_CIRCLE))
		{
			// open menu level
			SceneManager::GetInstance()->OpenLevel("Main Menu Scene");
		}
	}

	return true;
}

bool LevelThree::Render()
{
	LevelScene::Render();

	return false;
}

bool LevelThree::RenderUI()
{
	LevelScene::RenderUI();

	return false;
}

bool LevelThree::Cleanup()
{
	LevelScene::Cleanup();

	return false;
}
