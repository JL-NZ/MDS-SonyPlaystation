#include <gnmx.h>
#include <video_out.h>
#include <time.h>
#include <chrono>

#include "common/allocator.h"
#include "api_gnm/toolkit/toolkit.h"

#include "Render.h"
#include "AudioManager.h"
#include "Model.h"
#include "Utils.h"
#include "TextLabel.h"
#include "PhysicsEngine.h"
#include "rigidbody.h"
#include "GameObject.h"
#include "camera.h"
#include "MainMenu.h"
#include "GameOver.h"
#include "LevelOne.h"
#include "LevelTwo.h"
#include "LevelThree.h"
#include "SceneManager.h"

using namespace sce;
using namespace sce::Gnmx;
using namespace std;

std::shared_ptr<ControllerContext> g_controllerContext;
SceUserServiceUserId g_userID;

int main()
{
	// Seed PRNG
	srand(static_cast<unsigned int>(time(nullptr)));

	// Initialize camera
	Camera::GetInstance()->Initialize(90.0f, // FOV
		static_cast<float>(Render::GetInstance()->kDisplayBufferWidth) / static_cast<float>(Render::GetInstance()->kDisplayBufferHeight), // Ratio
		0.1f, // Near 
		5000.0f // Far
	);

	// Initialize input
	g_controllerContext = std::make_shared<ControllerContext>();	

	// Initialize scenes
	std::shared_ptr<MainMenu> MainMenuScene = std::make_shared<MainMenu>();
	MainMenuScene->m_Name = "Main Menu Scene";
	MainMenuScene->m_controllerContext = g_controllerContext;
	std::shared_ptr<GameOver> GameOverScene = std::make_shared<GameOver>();
	GameOverScene->m_Name = "Game Over Scene";
	GameOverScene->m_controllerContext = g_controllerContext;
	std::shared_ptr<LevelOne> LevelOneScene = std::make_shared<LevelOne>();
	LevelOneScene->m_Name = "Level One Scene";	
	LevelOneScene->m_controllerContext = g_controllerContext;
	std::shared_ptr<LevelTwo> LevelTwoScene = std::make_shared<LevelTwo>();
	LevelTwoScene->m_Name = "Level Two Scene";
	LevelTwoScene->m_controllerContext = g_controllerContext;
	std::shared_ptr<LevelThree> LevelThreeScene = std::make_shared<LevelThree>();
	LevelThreeScene->m_Name = "Level Three Scene";
	LevelThreeScene->m_controllerContext = g_controllerContext;
	
	SceneManager::GetInstance()->m_Scenes.push_back(MainMenuScene);
	SceneManager::GetInstance()->m_Scenes.push_back(GameOverScene);
	SceneManager::GetInstance()->m_Scenes.push_back(LevelOneScene);
	SceneManager::GetInstance()->m_Scenes.push_back(LevelTwoScene);
	SceneManager::GetInstance()->m_Scenes.push_back(LevelThreeScene);

	SceneManager::GetInstance()->OpenLevel("Main Menu Scene");

	// Physics initialisation
	PhysicsEngine* pPhysics = PhysicsEngine::GetInstance();		
		
	sceUserServiceInitialize(NULL);
	int ret = sceUserServiceGetInitialUser(&g_userID);
	if (ret < SCE_OK)
	{
		printf("sceUserServiceGetInitialUser failed: 0x%08X\n", ret);
		return ret;
	}
	
	ret = g_controllerContext->initialize(g_userID);
	if (ret < SCE_OK)
	{
		printf("controller initialization failed: 0x%08X\n", ret);
		return ret;
	}		

	float fAngle = 0.0f;

	while(!SceneManager::GetInstance()->m_bExitGame)
	{
		/// Update loop
		SceneManager::GetInstance()->Update();

		// Update input stuff
		g_controllerContext->update();		

		// Update physics world
		pPhysics->Update(0.016);			

		/// Render loop
		Render::GetInstance()->StartRender();
		Render::GetInstance()->SetPipelineState();

		SceneManager::GetInstance()->Render();		

		Render::GetInstance()->EndRender();		

		// Tick Cleanup
		SceneManager::GetInstance()->Cleanup();
	}
	
	// Cleanup
	Render::GetInstance()->Destroy();
	PhysicsEngine::GetInstance()->Destroy();
	AudioManager::GetInstance()->Destroy();
	sceScreamStopSoundSystem();
	sceSysmoduleUnloadModule(SCE_SYSMODULE_NGS2);

	return 0;
}