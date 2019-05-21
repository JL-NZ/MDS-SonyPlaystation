#include "GameObject.h"
#include "rigidbody.h"
#include "Model.h"
#include "PhysicsEngine.h"

GameObject::GameObject(){}

GameObject::~GameObject(){}

Rigidbody* GameObject::GetRigidbody()const {
	return m_pRigidbody;
}

Model* GameObject::GetModel()const {
	return m_pModel;
}

// Draw the model
void GameObject::Render() {
	// Update position with rigidbody if one exists
	if (m_pRigidbody) {
		UpdateModelPosition();
	}
	if (m_pModel) {
		m_pModel->Draw(TextureType::GNF);
	}
}

// Sets the rotation and translation of the model to match that of the current rigidbody state
inline void GameObject::UpdateModelPosition() {
	// Get Rigidbody rotation and position information
	sce::PhysicsEffects::PfxTransform3 transform(m_pRigidbody->GetState().getOrientation(), m_pRigidbody->GetState().getPosition());
	sce::PhysicsEffects::PfxQuat quat = m_pRigidbody->GetState().getOrientation();
	sce::PhysicsEffects::PfxVector4 pfxRotation = quat.axisAngle(quat);
	m_pModel->angle = pfxRotation.getW();
	m_pModel->rotateAxis = Vector3(pfxRotation.getX(), pfxRotation.getY(), pfxRotation.getZ());

	sce::PhysicsEffects::PfxVector3 pfxVector = m_pRigidbody->GetState().getPosition();
	m_pModel->translation = Vector3(pfxVector.getX(), pfxVector.getY(), pfxVector.getZ());
}

// Update the game object state
void GameObject::Update(float _fDeltaTick) {
	// Do stuff
}

/// Terrain Object
TerrainObject::TerrainObject(const char* _kcTerrainFile, const char* _kcTextureFile) {
	// Load model
	m_pModel = new Model(ModelType::kTerrain, _kcTextureFile, Vector3::zero());
	m_pModel->genFetchShaderAndOffsetCache("/app0/NoTexshader_vv.sb", "/app0/NoTexshader_p.sb");

	// Create rigidbody
	m_pRigidbody = new Rigidbody(kRBLargeMesh);
	//m_pRigidbody

}

TerrainObject::~TerrainObject(){}

/// Ball Object
BallObject::BallObject(const char* _kcTextureFile) {
	// Load model
	m_pModel = new Model(ModelType::kSphere, _kcTextureFile, Vector3::zero());
	m_pModel->genFetchShaderAndOffsetCache("/app0/shader_vv.sb", "/app0/shader_p.sb");

	// Create rigidbody
	m_pRigidbody = new Rigidbody(RigidbodyType::kRBSphere);
	m_pRigidbody->GetState().setMotionType(sce::PhysicsEffects::ePfxMotionType::kPfxMotionTypeActive);
	
}

BallObject::~BallObject(){}