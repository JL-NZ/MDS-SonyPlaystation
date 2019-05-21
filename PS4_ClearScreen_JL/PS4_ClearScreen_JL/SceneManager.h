#ifndef __SCENEMANAGEMENT_H__
#define __SCENEMANAGEMENT_H__

class SceneManager {
	private:
		SceneManager();
		static SceneManager* s_pInstance;

	public:
		~SceneManager();
		static SceneManager* GetInstance();
		void Destroy();
};

#endif // !__SCENEMANAGEMENT_H__
