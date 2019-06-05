#include "SceneManager.h"
#include "Scene.h"

SceneManager* SceneManager::s_pInstance = nullptr;

SceneManager::SceneManager(){}

SceneManager::~SceneManager(){}

SceneManager* SceneManager::GetInstance() {
	if (!s_pInstance) {
		s_pInstance = new SceneManager();
	}

	return s_pInstance;
}

void SceneManager::Destroy() {
	if (s_pInstance) {
		delete s_pInstance;
		s_pInstance = nullptr;
	}
}

std::shared_ptr<Scene> SceneManager::GetCurrentScene() const
{
	return m_CurrentScene;
}

bool SceneManager::SetCurrentScene(std::shared_ptr<Scene> _scene)
{
	// Check if its valid to set _scene as the current scene
	if (_scene != nullptr)
	{
		if (_scene->m_bInitialized)
		{
			m_CurrentScene = _scene;
			return true;
		}					
	}

	return false;	
}

bool SceneManager::Update()
{
	m_CurrentScene->Update();

	return true;
}

bool SceneManager::Render()
{
	m_CurrentScene->Render();
	m_CurrentScene->RenderUI();

	return true;
}
