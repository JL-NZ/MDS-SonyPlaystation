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
	
	// Camera
	Camera camera(90.0f, static_cast<float>(Render::GetInstance()->kDisplayBufferWidth) / static_cast<float>(Render::GetInstance()->kDisplayBufferHeight), 0.1f, 5000.0f);

	// Test physics objects
	CubeObject cube(Vector3(100.0f, 0.5f, 100.0f), "/app0/cat.gnf");
	cube.SetPosition(Vector3(0.0f, -1.0f, 0.0f));
	BallObject ball("/app0/cat.gnf");
	ball.SetPosition(Vector3(0.0f, 5.0f, 0.0f));

	AudioManager::GetInstance()->Initialize();
	SceScreamSoundParams soundParams = AudioManager::GetInstance()->InitializeScreamParams();
	SceScreamSFXBlock2* soundBank = AudioManager::GetInstance()->LoadAudioBank("/app0/testbank.bnk");
		
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

	//AudioManager::GetInstance()->PlaySound(soundBank, "/app0/testbank.bnk", soundParams);

	float fAngle = 0.0f;

	while(true)
	{
		/// Update loop
		// Clock
		previousTime = currentTime;
		currentTime = std::chrono::high_resolution_clock::now();
		float fDeltaTick = static_cast<float>( std::chrono::duration_cast<std::chrono::seconds>(currentTime - previousTime).count());

		g_controllerContext.update();
		ball.Update(fDeltaTick);
		
		// Quik math
		sce::Vectormath::Scalar::Aos::Quat quat;
		//quat = quat.rotation(Vector3(0.0f, TORADIANS * fAngle,  0.0f), 0);
		

		// Move Camera
		sce::PhysicsEffects::PfxVector3 ballPosition = ball.GetRigidbody()->GetState().getPosition();
		camera.SetPosition(Vector3(ballPosition.getX(), ballPosition.getY(), ballPosition.getZ()) + Vector3(0.0f, 0.0f, -10.0f));
		camera.PointAt(Vector3(ballPosition.getX(), ballPosition.getY(), ballPosition.getZ()));

		// Physics
		pPhysics->Update(0.016);

		/// Render loop
		Render::GetInstance()->StartRender();
		Render::GetInstance()->SetPipelineState();

		cube.Render();
		ball.Render();

		TextManager->DrawText();

		Render::GetInstance()->EndRender();
		
	}
	
	// Cleanup
	Render::GetInstance()->Destroy();

	return 0;
}