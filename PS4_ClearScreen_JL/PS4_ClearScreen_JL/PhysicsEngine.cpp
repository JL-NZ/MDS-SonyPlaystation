#include "PhysicsEngine.h"
#include <new>
#include "rigidbody.h"

PhysicsEngine* PhysicsEngine::s_pInstance = nullptr;

PhysicsEngine::PhysicsEngine() {
	// Create default world
	CreateDefaultWorld();

}

PhysicsEngine::~PhysicsEngine() {
	m_pWorld->finalize();

	// Delete rigidbodies
	m_pWorld->~PfxRigidBodyWorld();
	SCE_PFX_UTIL_FREE(m_pWorldBuffer);
	SCE_PFX_UTIL_FREE(m_pPoolBuffer);

	// Delete world
	if (m_pWorld) {
		delete m_pWorld;
	}
}

PhysicsEngine* PhysicsEngine::GetInstance() {
	if (!s_pInstance) {
		s_pInstance = new PhysicsEngine();
	}

	return s_pInstance;
}

void PhysicsEngine::Destroy() {
	if (s_pInstance) {
		delete s_pInstance;
		s_pInstance = nullptr;
	}
}

inline void PhysicsEngine::CreateDefaultWorld() {
	// Initialise world parameters
	sce::PhysicsEffects::PfxRigidBodyWorldParam worldParameters;
	worldParameters.simulationFlag |= SCE_PFX_ENABLE_CONTACT_CACHE;
	poolBytes = sce::PhysicsEffects::PfxRigidBodyWorld::getRigidBodyWorldBytes(worldParameters);
	void* m_pPoolBuffer = SCE_PFX_UTIL_ALLOC(16, poolBytes);
	worldParameters.poolBytes = poolBytes;
	worldParameters.poolBuff = m_pPoolBuffer;

	// Initialise world
	void* m_pWorldBuffer = SCE_PFX_UTIL_ALLOC(16, sizeof(sce::PhysicsEffects::PfxRigidBodyWorld));
	m_pWorld = new(m_pWorldBuffer)
		sce::PhysicsEffects::PfxRigidBodyWorld(worldParameters);
	m_pWorld->initialize();
	
}

sce::PhysicsEffects::PfxRigidBodyWorld* PhysicsEngine::GetWorld()const {
	return m_pWorld;
}

void PhysicsEngine::Update(float _fDeltaTick) {
	// Test
	m_pWorld->simulate(); return;

	// Update time
	float fTime = _fDeltaTick + m_fExtraTime;
	// Determine substeps
	unsigned int uiSteps = static_cast<unsigned int>(floor(fTime / kPhysicsTimestep));
	m_fExtraTime = fTime - (uiSteps * kPhysicsTimestep);
	if (uiSteps > 0) {
		// Simulate Physics
		m_pWorld->simulateSubStep(uiSteps);
	}

}