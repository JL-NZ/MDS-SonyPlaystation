#include "MainMenu.h"
#include "TextLabel.h"
#include "Utils.h"
#include "GameObject.h"
#include "PhysicsEngine.h"
#include "rigidbody.h"
#include "SceneManager.h"
#include "Render.h"
#include "camera.h"
#include "AudioManager.h"


MainMenu::MainMenu()
{
}


MainMenu::~MainMenu()
{
}

bool MainMenu::Initialize()
{

	// Text initialisation
	TextManager = std::make_shared<TextLabel>();
	TextManager->Initialize();

	std::shared_ptr<Text> ScoreText = TextManager->AddText(
		Vector2(0.30f, 0.35f),// position
		Vector2(50.0f), // scale
		"Main Menu \n\n\n\n\n\n\nJasper & Kerry's Physics Game \n\n\n\n\n\n\nUse Left stick to move, collect the balls \n\n\n\n\n\n\nPress X to start \n\n\n\n\n\n\nPress Circle to exit."
	);

	m_TextVector.push_back(ScoreText);	

	m_bInitialized = true;

	return true;
}

bool MainMenu::Update(float _deltaTick)
{
	if (m_controllerContext->isButtonDown(0, BUTTON_CROSS))
	{
		// open next level
		SceneManager::GetInstance()->OpenLevel("Level One Scene");
	}
	else if (m_controllerContext->isButtonDown(0, BUTTON_CIRCLE))
	{
		// exit game, stop tick
		SceneManager::GetInstance()->m_bExitGame = true;
	}

	return true;
}

bool MainMenu::Render()
{
	return true;
}

bool MainMenu::RenderUI()
{
	// Render UI		
	TextManager->DrawText();

	return true;
}

bool MainMenu::Cleanup()
{
	return false;
}

bool MainMenu::ClosingLevel()
{
	return false;
}
