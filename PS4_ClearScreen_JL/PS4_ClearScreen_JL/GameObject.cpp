#include "GameObject.h"
#include "rigidbody.h"
#include "Model.h"
#include "PhysicsEngine.h"
#include "controller.h"
#include "camera.h"

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

// Process collisions with other objects
void GameObject::ProcessCollision(std::shared_ptr<GameObject> _otherObject, void* _userData)
{

}

// flag an object for destruction
void GameObject::Destroy(std::string _reason)
{
	std::string debugReason = _reason; // the reason for destruction
	m_bDestroy = true; // flag the object for destruction at the end of tick
}

// check if an object is flagged for destruction
bool GameObject::GetDestroyFlag()
{
	return m_bDestroy;
}

// Should only be called at the end of a tick, in the cleanup loop
void GameObject::FinishDestruction()
{
	
}

// Set the game object to be at a certain position
void GameObject::SetPosition(Vector3 _newPosition) {
	m_pModel->translation = _newPosition;
	// Update rigidbody correctly if one exists
	if (m_pRigidbody) {
		m_pRigidbody->GetState().setPosition(sce::PhysicsEffects::PfxVector3(_newPosition.getX(), _newPosition.getY(), _newPosition.getZ()));
	}
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
BallObject::BallObject(Vector3 _scale, const char* _kcTextureFile) {
	// Set type
	m_eType = ObjectType::eBALL;

	// Load model
	m_pModel = new Model(ModelType::kSphere, _kcTextureFile, Vector3::zero());
	m_pModel->scale = Vector3(0.5f);
	m_pModel->genFetchShaderAndOffsetCache("/app0/shader_vv.sb", "/app0/shader_p.sb");
	
	// Create collision shape
	sce::PhysicsEffects::PfxSphere sphere;
	sphere.m_radius = _scale.getX();

	// Create rigidbody	
	m_pRigidbody = new Rigidbody(RigidbodyType::kRBSphere);
	m_pRigidbody->GetState().setMotionType(sce::PhysicsEffects::ePfxMotionType::kPfxMotionTypeActive);
	m_pRigidbody->GetCollider().getShape(0).reset();
	m_pRigidbody->GetCollider().getShape(0).setSphere(sphere);
	m_pRigidbody->GetCollider().finish();	
	m_pRigidbody->GetState().setUserData(this);
}

BallObject::~BallObject(){}

void BallObject::Update(float _fDeltaTick) {
	// Get L stick input
	AnalogStick lStick = ControllerContext::GetInstance()->LeftStick;

	// Check that there is input from the player
	if (lStick.x != 0.0f && lStick.y != 0.0f) {
		// Give them new movement
		Vector3 direction = sce::Vectormath::Scalar::Aos::normalize(Camera::GetInstance()->m_CameraRight * -lStick.x + Camera::GetInstance()->m_CameraFront * -lStick.y) * m_fMoveSpeed;
		GetRigidbody()->GetState().setLinearVelocity(sce::PhysicsEffects::PfxVector3(direction.getX(), -9.81f, direction.getZ())*_fDeltaTick * m_fMoveSpeed) ;
	}
}

void BallObject::ProcessCollision(std::shared_ptr<GameObject> _otherObject, void* _userData)
{
	void* pUserData = _userData;
	if (pUserData)
	{
		GameObject* pObject = static_cast<GameObject*>(pUserData);
		ObjectType eObjectType = pObject->GetType();
		switch (eObjectType)
		{		
		case ObjectType::eCOLLECTABLE:
		{			
			_otherObject->Destroy("Ball has collided with collectable.");
			m_iScore += 1;
			break;
		}
		}
	}
}

CubeObject::CubeObject(Vector3 _scale, const char* _kcTextureFile) {
	// Set type
	m_eType = ObjectType::eCUBE;

	// Load model
	m_pModel = new Model(ModelType::kCube, _kcTextureFile, Vector3::zero());
	m_pModel->scale = _scale;
	m_pModel->genFetchShaderAndOffsetCache("/app0/shader_vv.sb", "/app0/shader_p.sb");

	// Create rigidbody
	m_pRigidbody = new Rigidbody(RigidbodyType::kRBBox);
	sce::PhysicsEffects::PfxBox box(sce::PhysicsEffects::PfxVector3(_scale.getX(), _scale.getY(), _scale.getZ()));
	m_pRigidbody->GetCollider().getShape(0).reset();
	m_pRigidbody->GetCollider().getShape(0).setBox(box);
	m_pRigidbody->GetCollider().finish();
	m_pRigidbody->GetState().setMotionType(sce::PhysicsEffects::ePfxMotionType::kPfxMotionTypeFixed);
	m_pRigidbody->GetState().setUserData(this);	
}

CubeObject::~CubeObject(){}

CollectableObject::CollectableObject(Vector3 _scale, const char * _kcTextureFile)
{
	// Set type
	m_eType = ObjectType::eCOLLECTABLE;

	// Load model
	m_pModel = new Model(ModelType::kSphere, _kcTextureFile, Vector3::zero());
	m_pModel->scale = _scale;
	m_pModel->genFetchShaderAndOffsetCache("/app0/shader_vv.sb", "/app0/shader_p.sb");

	// Create collision shape
	sce::PhysicsEffects::PfxSphere sphere;
	sphere.m_radius = _scale.getX();

	// Create rigidbody	
	m_pRigidbody = new Rigidbody(RigidbodyType::kRBSphere);
	m_pRigidbody->GetState().setMotionType(sce::PhysicsEffects::ePfxMotionType::kPfxMotionTypeFixed);
	m_pRigidbody->GetCollider().getShape(0).reset();
	m_pRigidbody->GetCollider().getShape(0).setSphere(sphere);
	m_pRigidbody->GetCollider().finish();
	m_pRigidbody->GetState().setUserData(this);
}

CollectableObject::~CollectableObject(){}
