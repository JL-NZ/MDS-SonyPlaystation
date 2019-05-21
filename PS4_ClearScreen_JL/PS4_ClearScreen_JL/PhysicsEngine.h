#ifndef __PHYSICSENGINE_H__
#define __PHYSICSENGINE_H__

// Local includes
#include "physicseffects/pfx_high_level_include.h"
#include "physicseffects/pfx_rigidbody_world.h"
#include "physicseffects/pfx_rigidbody_world_implementation.h"

class PhysicsEngine {
	public:
		~PhysicsEngine();

		static PhysicsEngine* GetInstance();
		void Destroy();

		sce::PhysicsEffects::PfxRigidBodyWorld* GetWorld()const;

	private:
		// Singleton enforcement
		PhysicsEngine();
		static PhysicsEngine* s_pInstance;

		// Member variables
		sce::PhysicsEffects::PfxRigidBodyWorld* m_pWorld;
		void* m_pWorldBuffer;
		void* m_pPoolBuffer;

		// Internal helper function
		inline void CreateDefaultWorld();


};

#endif // !__PHYSICSENGINE_H__
