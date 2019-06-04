#include "LevelScene.h"



LevelScene::LevelScene()
{	
	printf("LevelScene::LevelScene \n");
}

LevelScene::~LevelScene()
{
	printf("LevelScene::~LevelScene \n");
}

bool LevelScene::Initialize()
{
	printf("LevelScene::Initialize \n");
	m_bInitialized = true;
	return true;
}

bool LevelScene::Update()
{
	printf("LevelScene::Update \n");
	return true;
}

bool LevelScene::Render()
{
	printf("LevelScene::Render \n");
	return true;
}

bool LevelScene::RenderUI()
{
	printf("LevelScene::RenderUI \n");
	return true;
}
