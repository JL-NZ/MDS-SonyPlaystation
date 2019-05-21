#pragma once

#ifndef __SCENEMANAGEMENT_H__
#define __SCENEMANAGEMENT_H__

// Local Includes

// Library Includes
#include <string>
#include <memory>
#include <vector>

class SceneManager {
	// Singleton
	private:
		SceneManager();
		static SceneManager* s_pInstance;

	public:
		~SceneManager();
		static SceneManager* GetInstance();
		void Destroy();
	//

public:
	//// Public Variables
	std::vector<std::shared_ptr<class Scene>> m_Scenes;	// Container for scenes
	float m_fDeltaTime = 0.0f;	// Should be updated every frame with the current delta time

	//// Public Functions
	std::shared_ptr<class Scene> GetCurrentScene() const; // Returns a ptr to the current scene
	bool SetCurrentScene(std::shared_ptr<class Scene> _scene); // Sets the current scene, scene must not be nullptr and must be already initialized
	bool Update(); // Updates the current scene
	bool Render(); // Renders the current scene

private:
	//// Private Variables
	std::shared_ptr<class Scene> m_CurrentScene = nullptr;

	//// Private Functions


};

#endif // !__SCENEMANAGEMENT_H__
