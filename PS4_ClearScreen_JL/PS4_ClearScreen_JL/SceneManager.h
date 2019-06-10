#pragma once

#ifndef __SCENEMANAGEMENT_H__
#define __SCENEMANAGEMENT_H__

// Local Includes

// Library Includes
#include <string>
#include <memory>
#include <vector>
#include <time.h>
#include <chrono>

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
	std::chrono::high_resolution_clock::time_point previousTime = (std::chrono::high_resolution_clock::now());
	std::chrono::high_resolution_clock::time_point currentTime = (std::chrono::high_resolution_clock::now());
	std::chrono::high_resolution_clock::duration deltaTime;
	std::chrono::high_resolution_clock::duration timeElapsed;
	bool m_bExitGame = false;

	//// Public Functions
	std::shared_ptr<class Scene> GetCurrentScene() const; // Returns a ptr to the current scene
	
	bool Update(); // Updates the current scene
	bool Render(); // Renders the current scene
	bool Cleanup(); // Cleans up destroyed entities
	bool OpenLevel(std::string _levelName); // opens a level of the specified name if it exists, sets it as the current scene

private:
	//// Private Variables
	std::shared_ptr<class Scene> m_CurrentScene = nullptr;

	//// Private Functions
	bool SetCurrentScene(std::shared_ptr<class Scene> _scene); // Sets the current scene, scene must not be nullptr and must be already initialized

};

#endif // !__SCENEMANAGEMENT_H__
