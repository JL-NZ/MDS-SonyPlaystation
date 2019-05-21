#ifndef __RIGIDBODY_H__
#define __RIGIDBODY_H__

// Local Includes
#include "physicseffects/pfx_high_level_include.h"
#include "physicseffects/pfx_rigidbody_world.h"
#include "physicseffects/pfx_rigidbody_world_implementation.h"

enum RigidbodyType {
	kRBSphere = 0,
	kRBBox,
	kRBCylinder,
	kRBCapulse,
	kRBMesh,
	kRBCompound,
	kRBLargeMesh
};

class Rigidbody {
	private:
		RigidbodyType m_eRBType;
		sce::PhysicsEffects::PfxUInt32 m_uiID;
		sce::PhysicsEffects::PfxRigidState m_State;
		sce::PhysicsEffects::PfxRigidBody m_Body;
		sce::PhysicsEffects::PfxShape m_Shape;
		sce::PhysicsEffects::PfxCollidable m_Collider;

		// Internal helper functions
		inline void DetermineShape();
		inline void DetermineInertia();

	public:
		Rigidbody(RigidbodyType _eType);
		~Rigidbody();
};

#endif // !__RIGIDBODY_H__
