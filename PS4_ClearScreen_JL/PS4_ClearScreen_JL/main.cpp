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
#include "controller.h"
#include "TextLabel.h"
#include "PhysicsEngine.h"
#include "rigidbody.h"
#include "GameObject.h"
#include "camera.h"
#include "LevelOne.h"
#include "LevelTwo.h"
#include "LevelThree.h"
#include "SceneManager.h"

using namespace sce;
using namespace sce::Gnmx;
using namespace std;

ControllerContext g_controllerContext;
SceUserServiceUserId g_userID;

int main()
{
	// Initialize sound stuff
	AudioManager::GetInstance()->Initialize();

	std::shared_ptr<LevelOne> LevelOneScene = std::make_shared<LevelOne>();
	LevelOneScene->Initialize();
	LevelOneScene->m_Name = "Level One Scene";
	SceneManager::GetInstance()->m_Scenes.push_back(LevelOneScene);
	SceneManager::GetInstance()->SetCurrentScene(LevelOneScene);

	// Physics initialisation
	PhysicsEngine* pPhysics = PhysicsEngine::GetInstance();		
		
	sceUserServiceInitialize(NULL);
	int ret = sceUserServiceGetInitialUser(&g_userID);
	if (ret < SCE_OK)
	{
		printf("sceUserServiceGetInitialUser failed: 0x%08X\n", ret);
		return ret;
	}
	
	ret = g_controllerContext.initialize(g_userID);
	if (ret < SCE_OK)
	{
		printf("controller initialization failed: 0x%08X\n", ret);
		return ret;
	}		

	float fAngle = 0.0f;

	while(true)
	{
		/// Update loop
		SceneManager::GetInstance()->Update();

		// Update input stuff
		g_controllerContext.update();		

		// Update physics world
		pPhysics->Update(0.016);		

		// Quik math
		sce::Vectormath::Scalar::Aos::Quat quat;
		//quat = quat.rotation(Vector3(0.0f, TORADIANS * fAngle,  0.0f), 0);				

		/// Render loop
		Render::GetInstance()->StartRender();
		Render::GetInstance()->SetPipelineState();

		SceneManager::GetInstance()->Render();		

		Render::GetInstance()->EndRender();		
	}
	
	// Cleanup
	Render::GetInstance()->Destroy();
	PhysicsEngine::GetInstance()->Destroy();
	AudioManager::GetInstance()->Destroy();
	sceScreamStopSoundSystem();
	sceSysmoduleUnloadModule(SCE_SYSMODULE_NGS2);

	return 0;
}