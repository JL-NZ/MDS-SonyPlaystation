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

	// Initial collectables
	for (unsigned int i = 0; i < 7; ++i) {
		AddCollectable();
	}

	// Level time
	m_fLevelTime = 15.0f;

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
