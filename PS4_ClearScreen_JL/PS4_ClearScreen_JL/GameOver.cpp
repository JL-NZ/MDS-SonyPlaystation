#include "GameOver.h"
#include "TextLabel.h"
#include "Utils.h"
#include "GameObject.h"
#include "PhysicsEngine.h"
#include "rigidbody.h"
#include "SceneManager.h"
#include "Render.h"
#include "camera.h"


GameOver::GameOver()
{
}


GameOver::~GameOver()
{
}

bool GameOver::Initialize()
{	
	// Text initialisation
	TextManager = std::make_shared<TextLabel>();
	TextManager->Initialize();

	std::shared_ptr<Text> ScoreText = TextManager->AddText(
		Vector2(0.30f, 0.35f),// position
		Vector2(50.0f), // scale
		"                   Game Over \n\n\n\n\n\n\n        Thank you for playing. \n\n\n\n\n\n\n      Press X to return to the menu \n\n\n\n\n\n\n         Press Circle to exit."
	);

	m_TextVector.push_back(ScoreText);

	m_bInitialized = true;

	return true;
}

bool GameOver::Update(float _deltaTick)
{
	if (m_controllerContext->isButtonDown(0, BUTTON_CROSS))
	{
		// open next level
		SceneManager::GetInstance()->OpenLevel("Main Menu Scene");
	}
	else if (m_controllerContext->isButtonDown(0, BUTTON_CIRCLE))
	{
		// exit game, stop tick
		SceneManager::GetInstance()->m_bExitGame = true;
	}

	return true;
}

bool GameOver::Render()
{
	return true;
}

bool GameOver::RenderUI()
{
	// Render UI		
	TextManager->DrawText();

	return true;
}

bool GameOver::Cleanup()
{
	return true;
}
