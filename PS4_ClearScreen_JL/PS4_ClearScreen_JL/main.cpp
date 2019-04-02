#include <gnmx.h>
#include <video_out.h>

#include "common/allocator.h"
#include "api_gnm/toolkit/toolkit.h"

#include "Render.h"
#include "Model.h"
#include "Utils.h"
#include "controller.h"


using namespace sce;
using namespace sce::Gnmx;

Controller::Input::ControllerContext g_controllerContext;
SceUserServiceUserId g_userID;

int main()
{
	Model* Model0 = new Model(ModelType::kSphere, Vector3(0.0f, 0.0f, 0.0f), Vector3(2.0f, 2.0f, 2.0f), Vector3(0.0f, 1.0f, 0.0f), 0.0f);
	Model* Model1 = new Model(ModelType::kCube, Vector3(-10.0f, 0.0f, 0.0f), Vector3(2.0f, 2.0f, 2.0f), Vector3(0.0f, 1.0f, 0.0f), 0.0f);
	
	Model0->genFetchShaderAndOffsetCache("/app0/shader_vv.sb", "/app0/shader_p.sb");
	Model1->genFetchShaderAndOffsetCache("/app0/shader_vv.sb", "/app0/shader_p.sb");
		
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
		//printf("%d \n", frameIndex);
		g_controllerContext.update(frameIndex / 1000);

		Render::GetInstance()->StartRender();
		Render::GetInstance()->SetPipelineState();
			
		Model0->Draw();
		Model1->Draw();

		Render::GetInstance()->EndRender();

		if (g_controllerContext.isButtonDown(0, Controller::Input::BUTTON_CROSS, Controller::Input::PATTERN_ANY))
		{
			printf("Button down");
		}
	}
	
	Render::GetInstance()->Destroy();

	return 0;
}