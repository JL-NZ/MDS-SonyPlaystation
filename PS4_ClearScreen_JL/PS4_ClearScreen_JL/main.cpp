#include <gnmx.h>
#include <video_out.h>

#include "common/allocator.h"
#include "api_gnm/toolkit/toolkit.h"

#include "Render.h"
#include "AudioManager.h"
#include "Model.h"
#include "Utils.h"
#include "controller.h"
#include "CCamera.h"
#include "TextLabel.h"
#include "SceneManager.h"
#include "LevelScene.h"

using namespace sce;
using namespace sce::Gnmx;
using namespace std;

ControllerContext g_controllerContext;
SceUserServiceUserId g_userID;

int main()
{
	// Working to implement the new scenemanager/scene/levelscene etc stuff here
	std::shared_ptr<LevelScene> LvlScene = std::make_shared<LevelScene>();
	LvlScene->Initialize();

	SceneManager* SceneMgr = SceneManager::GetInstance();
	SceneMgr->m_Scenes.push_back(LvlScene);
	SceneMgr->SetCurrentScene(LvlScene);
	//...

	TextLabel* Text = new TextLabel();
	Text->Initialize();
	Text->AddText(Vector3(0.5f, 0.5f, 0.0f), "blahblah");
	Text->AddText(Vector3(0.5f, 0.6f, 0.0f), "blahblah");
	

	Model* SphereModel = new Model(ModelType::kSphere, "/app0/mytextures.gnf", Vector3(5.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f), 0.0f);
	Model* CubeModel = new Model(ModelType::kCube, "/app0/cat.gnf", Vector3(-5.0f, 0.0f, 0.0f), Vector3(2.0f, 2.0f, 2.0f), Vector3(1.0f, 0.0f, 0.0f), 0.0f);
	Model* Model2 = new Model(ModelType::kTriangle, "/app0/normalmap.gnf", Vector3(-10.0f, 0.0f, 0.0f), Vector3(2.0f, 2.0f, 2.0f), Vector3(0.0f, 1.0f, 0.0f), 180.0f);
	Model* Model3 = new Model(ModelType::kQuad, "/app0/kanna.gnf", Vector3(10.0f, 0.0f, 0.0f), Vector3(2.0f, 2.0f, 2.0f), Vector3(0.0f, 1.0f, 0.0f), 180.0f);
	Model* CubeMap = new Model(ModelType::kCube, "/app0/cubemap3.gnf", Vector3(0.0f, 0.0f, 0.0f), Vector3(1000.0f, 1000.0f, 1000.0f), Vector3(0.0f, 0.0f, 1.0f), 0.0f);
	Model* TerrainModel = new Model(ModelType::kTerrain, "/app0/cat.gnf", Vector3(0.0f, -100.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f), Vector3(0.0f, 0.0f, 1.0f), 0.0f);
	
	SphereModel->genFetchShaderAndOffsetCache("/app0/shader_vv.sb", "/app0/shader_p.sb");
	CubeModel->genFetchShaderAndOffsetCache("/app0/shader_vv.sb", "/app0/shader_p.sb");
	Model2->genFetchShaderAndOffsetCache("/app0/shader_vv.sb", "/app0/shader_p.sb");
	Model3->genFetchShaderAndOffsetCache("/app0/shader_vv.sb", "/app0/shader_p.sb");
	TerrainModel->genFetchShaderAndOffsetCache("/app0/NoTexshader_vv.sb", "/app0/NoTexshader_p.sb");
	CubeMap->genFetchShaderAndOffsetCache("/app0/CMshader_vv.sb", "/app0/CMshader_p.sb");

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

	for (uint32_t frameIndex = 0; frameIndex < 10000; ++frameIndex)
	{	
		// Working to implement scene heirarchy stff here
		SceneMgr->m_fDeltaTime = 0.0f; // ideally we set delta time here properly
		SceneMgr->Update(); 
		SceneMgr->Render();

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
				SphereModel->angle -= 0.01f;
			}
			if (g_controllerContext.isButtonDown(0, BUTTON_RIGHT))
			{
				SphereModel->angle += 0.01f;
			}

			// Object two rotation
			if (g_controllerContext.isButtonDown(0, BUTTON_UP))
			{
				CubeModel->angle -= 0.01f;
			}
			if (g_controllerContext.isButtonDown(0, BUTTON_DOWN))
			{
				CubeModel->angle += 0.01f;
			}
		}		
		g_controllerContext.update();		

		Render::GetInstance()->StartRender();
		Render::GetInstance()->SetPipelineState();
			
		SphereModel->Draw(TextureType::GNF);
		CubeModel->Draw(TextureType::GNF);
		Model2->Draw(TextureType::GNF);
		Model3->Draw(TextureType::GNF);					

		Render::GetInstance()->ToggleBackfaceCulling(false);
		TerrainModel->Draw(TextureType::GNF);
		CubeMap->Draw(TextureType::GNF);
		Render::GetInstance()->ToggleBackfaceCulling(true);
		Text->DrawText();

		Render::GetInstance()->EndRender();
	}
	
	// Cleanup
	CCamera::GetInstance()->Destroy();
	Render::GetInstance()->Destroy();
	SceneManager::GetInstance()->Destroy();

	return 0;
}