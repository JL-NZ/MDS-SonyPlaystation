#include "LevelOne.h"
#include "TextLabel.h"
#include "Utils.h"
#include "GameObject.h"
#include "SceneManager.h"
#include "AudioManager.h"

#include <memory>


LevelOne::LevelOne()
{
}


LevelOne::~LevelOne()
{
}

bool LevelOne::Initialize()
{	
	LevelScene::Initialize();

	// Create initial collectables
	for (unsigned int i = 0; i < 3; ++i) {
		AddCollectable();
	}

	// Set level time
	m_fLevelTime = 30.0f;

	return true;
}

bool LevelOne::Update(float _deltaTick)
{
	LevelScene::Update(_deltaTick);

	if (m_bLevelComplete)
	{
		if (m_controllerContext->isButtonDown(0, BUTTON_CROSS))
		{
			// open next level
			SceneManager::GetInstance()->OpenLevel("Level Two Scene");
		}
		else if (m_controllerContext->isButtonDown(0, BUTTON_CIRCLE))
		{
			// open menu level
			SceneManager::GetInstance()->OpenLevel("Main Menu Scene");
		}
	}

	return true;
}

bool LevelOne::Render()
{
	LevelScene::Render();

	return true;
}

bool LevelOne::RenderUI()
{
	LevelScene::RenderUI();

	return true;
}

bool LevelOne::Cleanup()
{
	LevelScene::Cleanup();

	return false;
}
