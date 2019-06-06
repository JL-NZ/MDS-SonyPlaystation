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
#include "CCamera.h"
#include "TextLabel.h"
#include "PhysicsEngine.h"
//#include "clock.h"
#include "GameObject.h"

using namespace sce;
using namespace sce::Gnmx;
using namespace std;

ControllerContext g_controllerContext;
SceUserServiceUserId g_userID;

int main()
{
	std::chrono::high_resolution_clock::time_point previousTime = std::chrono::high_resolution_clock::now();;
	std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();;

	// Physics initialisation
	PhysicsEngine* pPhysics = PhysicsEngine::GetInstance();
	TextLabel* TextManager = new TextLabel();
	TextManager->Initialize();
	TextManager->AddText(Vector3(0.5f, 0.5f, 0.0f), "Score");
	TextManager->AddText(Vector3(0.5f, 0.6f, 0.0f), "Timer");
	

	// Test physics objects
	CubeObject cube(Vector3(10.0f, 0.5f, 10.0f), "/app0/cat.gnf");
	cube.SetPosition(Vector3(0.0f, -1.0f, 0.0f));
	BallObject ball("/app0/cat.gnf");
	ball.SetPosition(Vector3(0.0f, 5.0f, 0.0f));		

	AudioManager::GetInstance()->Initialize();
	SceScreamSoundParams BGMsoundParams = AudioManager::GetInstance()->InitializeScreamParams();
	BGMsoundParams.gain = 0.25f;	
	SceScreamSoundParams soundParams = AudioManager::GetInstance()->InitializeScreamParams();	
	SceScreamSFXBlock2* soundBank = AudioManager::GetInstance()->LoadAudioBank("/app0/SoundBank.bnk");
		
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

	AudioManager::GetInstance()->PlaySound(soundBank, "bgm.wav", BGMsoundParams);	

	while(true)
		{
		// Clock
		previousTime = currentTime;
		currentTime = std::chrono::high_resolution_clock::now();
		float fDeltaTick = static_cast<float>( std::chrono::duration_cast<std::chrono::seconds>(currentTime - previousTime).count());
		//printf("time: %f \n", fDeltaTick);		

		// Camera
		{
			// Camera movement
			{
				CCamera* Camera = CCamera::GetInstance();


				// Forward Component
				float fXForward = Camera->GetForwardVector().getX() * -g_controllerContext.LeftStick.y;
				float fYForward = Camera->GetForwardVector().getY() * -g_controllerContext.LeftStick.y;
				float fZForward = Camera->GetForwardVector().getZ() * -g_controllerContext.LeftStick.y;

				// Right component
				float fXRight = Camera->GetRightVector().getX() * -g_controllerContext.LeftStick.x;
				float fYRight = Camera->GetRightVector().getY() * -g_controllerContext.LeftStick.x;
				float fZRight = Camera->GetRightVector().getZ() * -g_controllerContext.LeftStick.x;

				// Combined position difference
				float fXCombined = fXForward + fXRight;
				float fYCombined = fYForward + fYRight;
				float fZCombined = fZForward + fZRight;

				Vector3 vec3_Final = Vector3(fXCombined, fYCombined, fZCombined);

				float fCameraSpeed = 0.1f;
				vec3_Final *= fCameraSpeed;

				// New final position
				CCamera::GetInstance()->m_vec3_CameraPos += vec3_Final;
			}

			// Camera rotation
			{
				// Up/Down Rotation
				CCamera::GetInstance()->m_fYAngle -= (g_controllerContext.RightStick.y / 40.0f);

				// Left/Right Rotation
				CCamera::GetInstance()->m_fXAngle -= (g_controllerContext.RightStick.x / 40.0f);
			}

			CCamera::GetInstance()->Process();
		}

		// Object rotation
		{
			// Object one rotation
			if (g_controllerContext.isButtonDown(0, BUTTON_LEFT))
			{
				AudioManager::GetInstance()->PlaySound(soundBank, "pop.wav", soundParams);
				
			}
			if (g_controllerContext.isButtonDown(0, BUTTON_RIGHT))
			{
				AudioManager::GetInstance()->PlaySound(soundBank, "roll.wav", soundParams);
				
			}

			// Object two rotation
			if (g_controllerContext.isButtonDown(0, BUTTON_UP))
			{
				AudioManager::GetInstance()->PlaySound(soundBank, "cheer.wav", soundParams);
				
			}
			if (g_controllerContext.isButtonDown(0, BUTTON_DOWN))
			{
				
			}
		}
		g_controllerContext.update();

		// Physics
		pPhysics->Update(0.015f);

		// Render loop
		Render::GetInstance()->StartRender();
		Render::GetInstance()->SetPipelineState();

		cube.Render();
		ball.Render();

		//SphereModel->Draw(TextureType::GNF);
		//CubeModel->Draw(TextureType::GNF);
		//Model2->Draw(TextureType::GNF);
		//Model3->Draw(TextureType::GNF);		

		//Render::GetInstance()->ToggleBackfaceCulling(false);
		//TerrainModel->Draw(TextureType::GNF);
		//CubeMap->Draw(TextureType::GNF);
		//Render::GetInstance()->ToggleBackfaceCulling(true);
		TextManager->DrawText();

		Render::GetInstance()->EndRender();
		
	}
	
	// Cleanup
	CCamera::GetInstance()->Destroy();
	Render::GetInstance()->Destroy();
	sceScreamStopSoundSystem();
	sceSysmoduleUnloadModule(SCE_SYSMODULE_NGS2);

	return 0;
}