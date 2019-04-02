#include <gnmx.h>
#include <video_out.h>

#include "common/allocator.h"
#include "api_gnm/toolkit/toolkit.h"

#include "Render.h"
#include "Model.h"
#include "Utils.h"
#include "controller.h"
#include "CCamera.h"


using namespace sce;
using namespace sce::Gnmx;

ControllerContext g_controllerContext;
SceUserServiceUserId g_userID;

int main()
{
	Model* Model0 = new Model(ModelType::kSphere, "/app0/mytextures.gnf", Vector3(0.0f, 0.0f, 0.0f), Vector3(2.0f, 2.0f, 2.0f), Vector3(0.0f, 1.0f, 0.0f), 0.0f);
	Model* Model1 = new Model(ModelType::kCube, "/app0/cubemap2.gnf", Vector3(0.0f, 0.0f, 0.0f), Vector3(1000.0f, 1000.0f, 1000.0f), Vector3(0.0f, 0.0f, 1.0f), 0.0f);
	
	Model0->genFetchShaderAndOffsetCache("/app0/shader_vv.sb", "/app0/shader_p.sb");
	Model1->genFetchShaderAndOffsetCache("/app0/CMshader_vv.sb", "/app0/CMshader_p.sb");
		
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
	
	for (uint32_t frameIndex = 0; frameIndex < 10000; ++frameIndex)
	{	
		// Camera
		{
			// Camera movement
			{
				// Move left
				///Vector3 newPos = CCamera::GetInstance()->m_vec3_CameraPos;
				///newPos.setX(newPos.getX() - 0.2f);
				///CCamera::GetInstance()->m_vec3_CameraPos = newPos;

				// Move right
				///Vector3 newPos = CCamera::GetInstance()->m_vec3_CameraPos;
				///newPos.setX(newPos.getX() + 0.2f);
				///CCamera::GetInstance()->m_vec3_CameraPos = newPos;

				// Move up
				///Vector3 newPos = CCamera::GetInstance()->m_vec3_CameraPos;
				///newPos.setY(newPos.getY() + 0.2f);
				///CCamera::GetInstance()->m_vec3_CameraPos = newPos;

				// Move down
				///Vector3 newPos = CCamera::GetInstance()->m_vec3_CameraPos;
				///newPos.setY(newPos.getY() - 0.2f);
				///CCamera::GetInstance()->m_vec3_CameraPos = newPos;
			}

			// Camera rotation
			{
				// Rotate left
				///CCamera::GetInstance()->m_fTargetPosXAngle += 0.01f;

				// Rotate right
				CCamera::GetInstance()->m_fTargetPosXAngle -= 0.01f;

				// Rotate up
				CCamera::GetInstance()->m_fTargetPosYAngle += 0.01f;

				// Rotate down
				///CCamera::GetInstance()->m_fTargetPosYAngle -= 0.01f;
			}

			CCamera::GetInstance()->Process();
		}
		
		
		//printf("%d \n", frameIndex);
		g_controllerContext.update();		

		Render::GetInstance()->StartRender();
		Render::GetInstance()->SetPipelineState();
			
		Model0->Draw();
		Model1->Draw();			

		Render::GetInstance()->EndRender();

		if (g_controllerContext.isButtonDown(0, BUTTON_CROSS, PATTERN_ANY))
		{
			//printf("Button down");
		}
		
		Vector2 posXZ = g_controllerContext.getLeftStick(0);
		
		printf("LeftStick X: %d \n", posXZ.getX());
		//printf("LeftStick Y: %f \n", g_controllerContext.getLeftStick(0).getY());
	}
	
	// Cleanup
	CCamera::GetInstance()->Destroy();
	Render::GetInstance()->Destroy();

	return 0;
}