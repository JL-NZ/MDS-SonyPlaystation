#include "SceneManager.h"

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