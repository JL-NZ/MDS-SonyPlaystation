#include "rigidbody.h"
#include "PhysicsEngine.h"

Rigidbody::Rigidbody(RigidbodyType _eType):m_eRBType(_eType) {
	// Create new shape
	DetermineShape();

	// Set shape to collidable
	m_Collider.reset();
	m_Collider.addShape(m_Shape);
	m_Collider.finish();

	// Set up rigidbody
	m_Body.reset();
	m_Body.setMass(1.0f);
	DetermineInertia();

	// Set state
	m_State.reset();
	m_State.setPosition(sce::PhysicsEffects::PfxVector3(0.0f));
	m_State.setMotionType(sce::PhysicsEffects::kPfxMotionTypeActive);
	m_State.setUseSleep(true);

	// Register with world
	m_uiID = PhysicsEngine::GetInstance()->GetWorld()->addRigidBody(m_State, m_Body, m_Collider);
}

Rigidbody::~Rigidbody(){
	// Remove registration
	PhysicsEngine::GetInstance()->GetWorld()->removeRigidBody(m_uiID);
}

void Rigidbody::DetermineShape() {
	switch (m_eRBType) {
		case kRBSphere: {
			sce::PhysicsEffects::PfxSphere sphere(0.5f);
			m_Shape.reset();
			m_Shape.setSphere(sphere);
			break;
		}

		default:break;
	}
}

void Rigidbody::DetermineInertia() {
	switch (m_eRBType) {
		case kRBSphere: {
			m_Body.setInertia(sce::PhysicsEffects::pfxCalcInertiaSphere(0.5f, 1.0f));
			break;
		}

		default:break;
	}
}