#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__

// Forward declaration
class Rigidbody;
class Model;

class GameObject {
	private:
		Model* m_pModel;
		Rigidbody* m_pRigidbody;

	public:
		GameObject();
		~GameObject();
};

#endif // !__GAMEOBJECT_H__
