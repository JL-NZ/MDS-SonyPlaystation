#pragma once

// Local Includes

// Library Includes
#include <string>
#include <memory>
#include <vector>

class Scene
{
public:
	// Constructor
	Scene();
	// Desctructor
	virtual ~Scene();

	//// Public Variables
	std::string m_Name = ""; 
	bool m_bInitialized = false;
	std::vector<std::shared_ptr<class GameObject>> m_GameObjects;
	std::vector<std::shared_ptr<class UserInterface>> m_UserInterfaces;
	std::vector<std::shared_ptr<class SceScreamSFXBlock2>> m_SoundBanks;

	//// Public Functions
	virtual bool Initialize() = 0; // Should be called once, after constructor (returns false if there's an issue)
	virtual bool Update() = 0; // called every frame (returns false if there's an issue)
	virtual bool Render() = 0; // called every frame, after Update (returns false if there's an issue)
	virtual bool RenderUI() = 0; // called every frame, after Render (returns false if there's an issue)	 

private:
	//// Private Variables

	//// Private Functions
	



};

