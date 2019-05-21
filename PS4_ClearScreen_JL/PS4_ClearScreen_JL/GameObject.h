#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__

// Forward declaration
class Rigidbody;
class Model;

class GameObject {
	protected:
		Model* m_pModel;
		Rigidbody* m_pRigidbody;

		inline void UpdateModelPosition();

	public:
		GameObject();
		virtual ~GameObject();

		void Render();
		void Update(float _fDeltaTick);

		Rigidbody* GetRigidbody()const;
		Model* GetModel()const;
};

// This class is used as the base 
class TerrainObject : public GameObject{
	private:

	public:
		TerrainObject(const char* _kcTerrainFile, const char* _kcTextureFile);
		~TerrainObject();


};

class BallObject : public GameObject {
	private:

	public:
		BallObject(const char* _kcTextureFile);
		~BallObject();
};

#endif // !__GAMEOBJECT_H__
