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
	// Clock
	previousTime = currentTime;
	currentTime = (std::chrono::high_resolution_clock::now());
	deltaTime = currentTime - previousTime;
	timeElapsed += deltaTime;
	m_fDeltaTime = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(deltaTime).count())/1000.0f;

	m_CurrentScene->Update(m_fDeltaTime);

	return true;
}

bool SceneManager::Render()
{
	m_CurrentScene->Render();
	m_CurrentScene->RenderUI();

	return true;
}

bool SceneManager::Cleanup()
{
	if (m_CurrentScene)
	{
		m_CurrentScene->Cleanup();
	}
	return false;
}

bool SceneManager::OpenLevel(std::string _levelName)
{
	for (int i = 0; i < m_Scenes.size(); i++)
	{
		if (m_Scenes[i]->m_Name == _levelName)
		{
			if (m_CurrentScene)
			{
				m_CurrentScene->ClosingLevel();
			}
			m_Scenes[i]->Initialize();			
			SetCurrentScene(m_Scenes[i]);
		}
	}

	return false;
}
