#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__

#include <gnmx.h>
#include <memory>
#include "Utils.h"

// For checking user data type
enum ObjectType
{
	eCUBE,
	eBALL,	
	eCOLLECTABLE
};

// Forward declaration
class Rigidbody;
class Model;

class GameObject {
	protected:
		Model* m_pModel = nullptr;
		Rigidbody* m_pRigidbody = nullptr;
		ObjectType m_eType;
		bool m_bDestroy = false; // flag for destruction

		inline void UpdateModelPosition();

	public:
		GameObject();
		virtual ~GameObject();

		void Render();
		virtual void Update(float _fDeltaTick);
		virtual void ProcessCollision(std::shared_ptr<GameObject> _otherObject, void* _userData);
		virtual void Destroy(std::string _reason);
		virtual bool GetDestroyFlag();
		virtual void FinishDestruction();

		Rigidbody* GetRigidbody()const;
		Model* GetModel()const;
		ObjectType& GetType() { return m_eType; }

		void SetPosition(Vector3 _newPosition);

		
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
		float m_fMoveSpeed = 20.0f;
		int m_iScore = 0.0f;

	public:
		BallObject(Vector3 _scale, const char* _kcTextureFile);
		~BallObject();

		void Update(float _fDeltaTick);
		void ProcessCollision(std::shared_ptr<GameObject> _otherObject, void* _userData);
		const int GetScore() { return m_iScore; }
};

class CubeObject : public GameObject {
	public:
		CubeObject(Vector3 _scale, const char* _kcTextureFile);
		~CubeObject();
};

class CollectableObject : public GameObject {
public:
	CollectableObject(Vector3 _scale, const char* _kcTextureFile);
	~CollectableObject();
};

#endif // !__GAMEOBJECT_H__
