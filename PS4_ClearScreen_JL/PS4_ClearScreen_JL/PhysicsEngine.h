#ifndef __PHYSICSENGINE_H__
#define __PHYSICSENGINE_H__

//const static unsigned int kPhysicsSubSteps = 3;
const static float kPhysicsTimestep = 0.016f;

// Local includes
#include "physicseffects/pfx_high_level_include.h"
#include "physicseffects/pfx_rigidbody_world.h"
#include "physicseffects/pfx_rigidbody_world_implementation.h"

// Forward declaration
class Rigidbody;

class PhysicsEngine {
	public:
		~PhysicsEngine();

		static PhysicsEngine* GetInstance();
		void Destroy();
		void Update(float _fDeltaTick);

		sce::PhysicsEffects::PfxRigidBodyWorld* GetWorld()const;
		void DeregisterRigidbody(sce::PhysicsEffects::PfxUInt32 _uiBodyIndex);
		const sce::PhysicsEffects::PfxQueryContactInfo* GetCollisionInfoFirstContact(int _indexA);
		const sce::PhysicsEffects::PfxQueryContactInfo* GetCollisionInfoNextContact(const sce::PhysicsEffects::PfxQueryContactInfo* _previousContact, int _indexA);
		const sce::PhysicsEffects::PfxQueryContactInfo* GetCollisionInfo(int _indexA, int _indexB);

	private:
		// Singleton enforcement
		PhysicsEngine();
		static PhysicsEngine* s_pInstance;

		// Member variables
		sce::PhysicsEffects::PfxRigidBodyWorld* m_pWorld;
		sce::PhysicsEffects::PfxUInt32 poolBytes;
		void* m_pWorldBuffer;
		void* m_pPoolBuffer;
		float m_fTimer = 0.0f;
		float m_fExtraTime = 0.0f;

		// Internal helper function
		inline void CreateDefaultWorld();
		

};

#endif // !__PHYSICSENGINE_H__
