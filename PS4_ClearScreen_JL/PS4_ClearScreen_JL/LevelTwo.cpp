#include "LevelTwo.h"
#include "TextLabel.h"
#include "Utils.h"
#include "GameObject.h"
#include "SceneManager.h"
#include "AudioManager.h"

#include <memory>


LevelTwo::LevelTwo()
{
}


LevelTwo::~LevelTwo()
{
}

bool LevelTwo::Initialize()
{
	LevelScene::Initialize();

	// Initial collectables
	for (unsigned int i = 0; i < 5; ++i) {
		AddCollectable();
	}

	// Level time
	m_fLevelTime = 20.0f;

	return true;
}

bool LevelTwo::Update(float _deltaTick)
{
	LevelScene::Update(_deltaTick);

	if (m_bLevelComplete)
	{
		if (m_controllerContext->isButtonDown(0, BUTTON_CROSS))
		{
			// open next level
			SceneManager::GetInstance()->OpenLevel("Level Three Scene");
		}
		else if (m_controllerContext->isButtonDown(0, BUTTON_CIRCLE))
		{
			// open menu level
			SceneManager::GetInstance()->OpenLevel("Main Menu Scene");
		}
	}

	return true;
}

bool LevelTwo::Render()
{
	LevelScene::Render();

	return true;
}

bool LevelTwo::RenderUI()
{
	LevelScene::RenderUI();

	return true;
}

bool LevelTwo::Cleanup()
{
	LevelScene::Cleanup();

	return false;
}
