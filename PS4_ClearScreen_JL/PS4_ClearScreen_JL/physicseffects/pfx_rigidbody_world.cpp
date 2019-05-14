/* SIE CONFIDENTIAL
 * PlayStation(R)4 Programmer Tool Runtime Library Release 05.008.001
 *                Copyright (C) 2016 Sony Interactive Entertainment Inc.
 *                                                All Rights Reserved.
 */

//#define SCE_PFX_USE_BOOKMARK
#define SCE_PFX_USE_PERFCOUNTER

#include "pfx_rigidbody_world.h"

namespace sce{
namespace PhysicsEffects{

void PfxRigidBodyWorld::printPairs(const char *msg,PfxBroadphasePair *pairs,int n)
{
	SCE_PFX_PRINTF("--------- %u : %s ----------\n",m_frame,msg);
	for(int p=0;p<n;p++) {
		SCE_PFX_PRINTF("pair%d %d,%d key %u\n",p,pfxGetObjectIdA(pairs[p]),pfxGetObjectIdB(pairs[p]),pfxGetKey(pairs[p]));
		if(p > 0 && pfxGetKey(pairs[p-1]) == pfxGetKey(pairs[p])) {
			SCE_PFX_ALWAYS_ASSERT(false);
		}
	}
}

PfxUInt32 PfxRigidBodyWorld::getRigidBodyWorldBytes(const PfxRigidBodyWorldParam &param)
{
	PfxUInt32 dataBytes = 128;
	PfxUInt32 workBytes = param.contextWorkBytes; // used in PfxRigidBodyContext

	//---------------------------------------------------------
	// Data
	dataBytes += SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(PfxRigidState) * param.maxRigidBodies);
	dataBytes += SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(PfxRigidBody)  * param.maxRigidBodies);
	dataBytes += SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(PfxCollidable) * param.maxRigidBodies);

	dataBytes += SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(PfxUInt32) * param.maxRigidBodies) * 3; // used in PfxPoolArray
	dataBytes += SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(PfxUInt32) * ((param.maxRigidBodies+31)>>5)) * 3; // used in PfxPoolArray

	dataBytes += SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(PfxShape)  * param.maxShapes);
	dataBytes += SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(PfxUInt32) * param.maxShapes); // used in PfxPoolArray
	dataBytes += SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(PfxUInt32) * ((param.maxShapes+31)>>5)); // used in PfxPoolArray
	
	dataBytes += SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(PfxJoint)          * param.maxJoints);
	dataBytes += SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(PfxConstraintPair) * param.maxJoints);
	dataBytes += SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(PfxUInt32) * param.maxJoints) * 2; // used in PfxPoolArray
	dataBytes += SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(PfxUInt32) * ((param.maxJoints+31)>>5)) * 2; // used in PfxPoolArray
	
	dataBytes += SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(PfxBroadphasePair) * param.maxContacts);
	dataBytes += SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(PfxBroadphasePair) * param.maxContacts);
	dataBytes += SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(PfxUInt32) * param.maxNonContactPairs);
	
	dataBytes += 2 * pfxBroadphaseProxyContainerQueryMem(param.maxRigidBodies); // used in PfxBroadphaseProxyContainer
	dataBytes += pfxContactContainerQueryMem(param.maxContacts,param.ccdWorkBytes); // used in PfxContactContainer
	dataBytes += pfxSimulationIslandQueryBytes(param.maxRigidBodies); // used in PfxSimulationIsland
	
	if(param.simulationFlag & SCE_PFX_ENABLE_CONTACT_CACHE) {
		dataBytes += SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(PfxUInt32) * param.maxRigidBodies); // m_cachedContactIdTable
		dataBytes += SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(PfxQueryContactInfo) * param.maxContacts); // m_cachedContactInfos
		dataBytes += SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(PfxBroadphasePair) * param.maxContacts); // m_cachedContactPairs
	}
	
	workBytes += sizeof(PfxBroadphasePair) * param.maxContacts;
	
	return dataBytes + workBytes;
}

///////////////////////////////////////////////////////////////////////////////
// World Constractor / Destructor

PfxRigidBodyWorld::PfxRigidBodyWorld(const PfxRigidBodyWorldParam &param) : 
	m_pool((PfxUInt8*)param.poolBuff,param.poolBytes),
	m_segmentWidth(param.segmentWidth),
	m_worldMin(param.worldMin),
	m_worldMax(param.worldMax),
	m_gravity(param.gravity),
	m_maxNonContactPairs(param.maxNonContactPairs),
	m_maxPairs(param.maxContacts),
	m_timeStep(param.timeStep),
	m_separateWeight(param.separateWeight),
	m_positionIteration(param.positionIteration),
	m_sleepCount(param.sleepCount),
	m_sleepVelocity(param.sleepVelocity),
	m_simulationFlag(param.simulationFlag),
	m_numWorkerThreads(param.numWorkerThreads),
	m_multiThreadFlag(param.multiThreadFlag),
	m_userJobManagerSequenceFactoryInterface(param.userJobManagerSequenceFactoryInterface),
	m_contextWorkBytes(param.contextWorkBytes),
	m_rigidbodyContextBuff(NULL)
{
	m_velocityIteration[0] = param.velocityIteration[0];
	m_velocityIteration[1] = param.velocityIteration[1];
	m_velocityIteration[2] = param.velocityIteration[2];

	SCE_PFX_ALWAYS_ASSERT_MSG(param.poolBytes >= getRigidBodyWorldBytes(param),"Can't create the world because the memory shortage.");

	m_states.initialize(&m_pool,param.maxRigidBodies),
	m_bodies.initialize(&m_pool,param.maxRigidBodies),
	m_collidables.initialize(&m_pool,param.maxRigidBodies),
	m_shapes.initialize(&m_pool,param.maxShapes),
	m_jointPairs.initialize(&m_pool,param.maxJoints),
	m_joints.initialize(&m_pool,param.maxJoints),

	m_pairsBuff[0] = (PfxBroadphasePair*)m_pool.allocate(sizeof(PfxBroadphasePair)*param.maxContacts);
	m_pairsBuff[1] = (PfxBroadphasePair*)m_pool.allocate(sizeof(PfxBroadphasePair)*param.maxContacts);
	m_nonContactPairs = (PfxUInt32*)m_pool.allocate(sizeof(PfxUInt32)*param.maxNonContactPairs);

	if(m_simulationFlag & SCE_PFX_ENABLE_CONTACT_CACHE) {
		m_cachedContactIdTable = (PfxUInt32*)m_pool.allocate(sizeof(PfxUInt32)*param.maxRigidBodies);
		m_cachedContactInfos = (PfxQueryContactInfo*)m_pool.allocate(sizeof(PfxQueryContactInfo)*param.maxContacts);
		m_cachedContactPairs = (PfxBroadphasePair*)m_pool.allocate(sizeof(PfxBroadphasePair)*param.maxContacts);
	}
	
	// Initialize context
	PfxInt32 ret = 0;
	
	m_rigidbodyContextBuff = m_pool.allocate(m_contextWorkBytes);
	PfxRigidBodyContextParam contextParam;
	
	if(m_segmentWidth > 0) {
		contextParam.segmentWidth = m_segmentWidth;
	}
	
	if(m_numWorkerThreads > 0) {
		contextParam.numWorkerThreads = m_numWorkerThreads;
		contextParam.multiThreadFlag = m_multiThreadFlag;
	}
	
	contextParam.userJobManagerSequenceFactoryInterface = m_userJobManagerSequenceFactoryInterface;

	ret = pfxRigidBodyContextInit(m_rigidbodyContext,contextParam,m_rigidbodyContextBuff,m_contextWorkBytes);
	SCE_PFX_ALWAYS_ASSERT_MSG(ret==SCE_PFX_OK,"Can't create rigid body context.");
	
	// Initialize containers
	PfxUInt32 broadphaseProxyContainerBytes = pfxBroadphaseProxyContainerQueryMem(param.maxRigidBodies);
	void *broadphaseProxyContainerBuff = m_pool.allocate(broadphaseProxyContainerBytes);
	ret = pfxBroadphaseProxyContainerInit(m_broadphaseProxyContainer[0],param.maxRigidBodies,broadphaseProxyContainerBuff,broadphaseProxyContainerBytes);
	SCE_PFX_ALWAYS_ASSERT_MSG(ret==SCE_PFX_OK,"Can't create broadphase proxy container.");
	
	broadphaseProxyContainerBuff = m_pool.allocate(broadphaseProxyContainerBytes);
	ret = pfxBroadphaseProxyContainerInit(m_broadphaseProxyContainer[1],param.maxRigidBodies,broadphaseProxyContainerBuff,broadphaseProxyContainerBytes);
	SCE_PFX_ALWAYS_ASSERT_MSG(ret==SCE_PFX_OK,"Can't create broadphase proxy container.");
	
	PfxUInt32 contactContainerBytes = pfxContactContainerQueryMem(param.maxContacts,param.ccdWorkBytes);
	void *contactContainerBuff = m_pool.allocate(contactContainerBytes);
	ret = pfxContactContainerInit(m_contactContainer,param.maxContacts,contactContainerBuff,contactContainerBytes);
	SCE_PFX_ALWAYS_ASSERT_MSG(ret==SCE_PFX_OK,"Can't create contact container.");
	
	PfxUInt32 islandContainerBytes = pfxSimulationIslandQueryBytes(param.maxRigidBodies);
	void *islandContainerBuff = m_pool.allocate(islandContainerBytes);
	ret = pfxSimulationIslandInit(m_simulationIsland,param.maxRigidBodies,islandContainerBuff,islandContainerBytes);
	SCE_PFX_ALWAYS_ASSERT_MSG(ret==SCE_PFX_OK,"Can't create simulation island container.");
}

PfxRigidBodyWorld::~PfxRigidBodyWorld()
{
	pfxBroadphaseProxyContainerTerm(m_broadphaseProxyContainer[0]);
	pfxBroadphaseProxyContainerTerm(m_broadphaseProxyContainer[1]);
	pfxContactContainerTerm(m_contactContainer);
	pfxSimulationIslandTerm(m_simulationIsland);
	pfxRigidBodyContextTerm(m_rigidbodyContext);
}

///////////////////////////////////////////////////////////////////////////////
// Initialize / Finalize

void PfxRigidBodyWorld::initialize()
{
	reset();
}

void PfxRigidBodyWorld::finalize()
{
}

///////////////////////////////////////////////////////////////////////////////
// Reset

void PfxRigidBodyWorld::reset()
{
	m_frame = 0;
	m_pairSwap = 1;
	m_numPairs[0] = m_numPairs[1] = 0;
	m_states.clear();
	m_bodies.clear();
	m_collidables.clear();
	m_shapes.clear();
	m_jointPairs.clear();
	m_joints.clear();
	m_numNonContactPairs = 0;

	m_areaMin = m_worldMin;
	m_areaMax = m_worldMax;

	pfxRigidBodyContextClear(m_rigidbodyContext);
	pfxBroadphaseProxyContainerClear(m_broadphaseProxyContainer[0]);
	pfxBroadphaseProxyContainerClear(m_broadphaseProxyContainer[1]);
	pfxContactContainerClear(m_contactContainer);
	pfxSimulationIslandClear(m_simulationIsland);

	if(m_simulationFlag & SCE_PFX_ENABLE_CONTACT_CACHE) clearContactCache();
}

void PfxRigidBodyWorld::reconstructRigidBodyContext(PfxInt32 segmentWidth,PfxUInt32 numWorkerThreads,PfxUInt32 multiThreadFlag)
{
	PfxInt32 ret = 0;
	
	ret = pfxRigidBodyContextTerm(m_rigidbodyContext);
	SCE_PFX_ALWAYS_ASSERT_MSG(ret==SCE_PFX_OK,"Can't terminate rigid body context.");
	
	PfxRigidBodyContextParam contextParam;
	
	if(segmentWidth > 0) {
		m_segmentWidth = segmentWidth;
	}
	
	if(numWorkerThreads > 1) {
		m_numWorkerThreads = numWorkerThreads;
		m_multiThreadFlag = multiThreadFlag;
	}
	else {
		m_numWorkerThreads = 1;
		m_multiThreadFlag = 0;
	}
	
	contextParam.segmentWidth = m_segmentWidth;
	contextParam.numWorkerThreads = m_numWorkerThreads;
	contextParam.multiThreadFlag = m_multiThreadFlag;
	contextParam.userJobManagerSequenceFactoryInterface = m_userJobManagerSequenceFactoryInterface;
	
	ret = pfxRigidBodyContextInit(m_rigidbodyContext,contextParam,m_rigidbodyContextBuff,m_contextWorkBytes);
	SCE_PFX_ALWAYS_ASSERT_MSG(ret==SCE_PFX_OK,"Can't create rigid body context.");
}

///////////////////////////////////////////////////////////////////////////////
// Simulation Method

void PfxRigidBodyWorld::applyForce()
{
	for(unsigned int i=0;i<m_states.length();i++) {
		if(!m_states.isRemoved(i)) {
			pfxApplyExternalForce(
				m_states[i],m_bodies[i],
				m_bodies[i].getMass() * m_gravity,
				PfxVector3(0.0f),
				m_timeStep);
		}
	}
}

void PfxRigidBodyWorld::updateProxies()
{
	PfxUpdateBroadphaseProxiesParam param;
	param.numRigidBodies = m_states.length();
	param.states = m_states.ptr();
	param.collidables = m_collidables.ptr();
	param.outOfWorldBehavior = SCE_PFX_OUT_OF_WORLD_BEHAVIOR_FIX_MOTION;
	param.worldMin = m_worldMin;
	param.worldMax = m_worldMax;
	param.timeStep = m_timeStep;
	param.flags = SCE_PFX_UPDATE_PROXY_FLAG_IGNORE_RESET;
	
	PfxInt32 ret;
	param.proxyContainer = &m_broadphaseProxyContainer[0];
	param.motionTypeFilter = 1<<kPfxMotionTypeFixed;
	ret = pfxUpdateBroadphaseProxies(m_rigidbodyContext,param);
	if(ret != SCE_PFX_OK) SCE_PFX_PRINTF("pfxUpdateBroadphaseProxies failed %d\n",ret);
	
	param.proxyContainer = &m_broadphaseProxyContainer[1];
	param.motionTypeFilter = SCE_PFX_MOTION_MASK_ALL&(~(1<<kPfxMotionTypeFixed));
	ret = pfxUpdateBroadphaseProxies(m_rigidbodyContext,param);
	if(ret != SCE_PFX_OK) SCE_PFX_PRINTF("pfxUpdateBroadphaseProxies failed %d\n",ret);
}

void PfxRigidBodyWorld::broadphase()
{
	m_pairSwap = 1-m_pairSwap;

	unsigned int &numPreviousPairs = m_numPairs[1-m_pairSwap];
	unsigned int &numCurrentPairs = m_numPairs[m_pairSwap];
	PfxBroadphasePair *previousPairs = m_pairsBuff[1-m_pairSwap];
	PfxBroadphasePair *currentPairs = m_pairsBuff[m_pairSwap];

	//J ブロードフェーズコンテナの更新
	//E Update broadphase container
	updateProxies();

	//J 交差ペア探索
	//E Find overlapped pairs
	{
		PfxFindOverlapPairsParam param;
		PfxFindOverlapPairsResult result;
		
		PfxUInt32 numPairsActiveActive = 0;

		// Active x Active
		{
			param.maxPairs = m_maxPairs;
			param.proxyContainerA = &m_broadphaseProxyContainer[1];
			param.proxyContainerB = &m_broadphaseProxyContainer[1];
			
			result.pairs = currentPairs;
			
			PfxInt32 ret = pfxFindOverlapPairs(m_rigidbodyContext,param,result);
			if(ret != SCE_PFX_OK) SCE_PFX_PRINTF("pfxFindOverlapPairs failed %d\n",ret);
			
			numPairsActiveActive = result.numPairs;
		}
		
		// Active x Fixed
		{
			param.maxPairs = m_maxPairs - numPairsActiveActive;
			param.proxyContainerA = &m_broadphaseProxyContainer[0];
			param.proxyContainerB = &m_broadphaseProxyContainer[1];
			
			result.pairs = currentPairs + numPairsActiveActive;
			
			PfxInt32 ret = pfxFindOverlapPairs(m_rigidbodyContext,param,result);
			if(ret != SCE_PFX_OK) SCE_PFX_PRINTF("pfxFindOverlapPairs failed %d\n",ret);

			numCurrentPairs = numPairsActiveActive + result.numPairs;
		}
	}

	//J ノンコンタクトペアとして指定されているペアを排除する
	//E Remove pairs defined as a non-contact pair
	{
		pfxSortPairs(m_rigidbodyContext,currentPairs,numCurrentPairs);

		PfxUInt32 i = 0,j = 0;
		PfxUInt32 numRemovedPairs = 0;
		while(i < m_numNonContactPairs && j < numCurrentPairs) {
			PfxUInt32 key1 = m_nonContactPairs[i];
			PfxUInt32 key2 = pfxGetKey(currentPairs[j]);

			if(key1 < key2) {
				i++;
			}
			else if(key1 > key2) {
				j++;
			}
			else {// key1 == key2
				pfxSetKey(currentPairs[j],SCE_PFX_SENTINEL_KEY);
				numRemovedPairs++;
				i++;
				j++;
			}
		}

		if(numRemovedPairs > 0) {
			pfxSortPairs(m_rigidbodyContext,currentPairs,numCurrentPairs);
			numCurrentPairs -= numRemovedPairs;
		}
	}

	//J ペアを整理
	//E Refine pairs
	{
		PfxRefinePairsParam param;
		param.maxPairs = m_maxPairs;
		param.states = m_states.ptr();
		param.currentPairs = currentPairs;
		param.numCurrentPairs = numCurrentPairs;
		param.previousPairs = previousPairs;
		param.numPreviousPairs = numPreviousPairs;
		param.contactContainer = &m_contactContainer;
		param.sortCurrentPairs = false;
		
		PfxRefinePairsResult result;
		
		PfxInt32 ret = pfxRefinePairs(m_rigidbodyContext,param,result);
		if(ret != SCE_PFX_OK) SCE_PFX_PRINTF("pfxRefinePairs failed %d\n",ret);
		
		numCurrentPairs = result.numPairs; // 最終的に保持されるペアの数
	}
}

void PfxRigidBodyWorld::collision()
{
	unsigned int numCurrentPairs = m_numPairs[m_pairSwap];
	PfxBroadphasePair *currentPairs = m_pairsBuff[m_pairSwap];

	//J 衝突検出
	//E Detect collisions
	{
		PfxDetectCollisionParam param;
		param.contactPairs = currentPairs;
		param.numContactPairs = numCurrentPairs;
		param.states = m_states.ptr();
		param.collidables = m_collidables.ptr();
		param.numRigidBodies = m_states.length();
		param.contactContainer = &m_contactContainer;
		param.timeStep = m_timeStep;
		
		PfxInt32 ret = pfxDetectCollision(m_rigidbodyContext,param);
		if(ret != SCE_PFX_OK) SCE_PFX_PRINTF("pfxDetectCollision failed %d\n",ret);
	}

	//J シミュレーションアイランド生成
	//E Create simulation islands
	{
		pfxSimulationIslandClear(m_simulationIsland);
		
		//E Submit contact pairs to the simulation island
		//J コンタクトペアをシミュレーションアイランドに登録
		PfxInt32 ret = pfxSimulationIslandAddPairs(m_simulationIsland,currentPairs,numCurrentPairs);
		if(ret != SCE_PFX_OK) SCE_PFX_PRINTF("pfxSimulationIslandAddPairs failed %d\n",ret);

		//E Submit joint pairs to the simulation island
		//J ジョイントペアをシミュレーションアイランドに登録
		ret = pfxSimulationIslandAddPairs(m_simulationIsland,m_jointPairs.ptr(),m_jointPairs.length());
		if(ret != SCE_PFX_OK) SCE_PFX_PRINTF("pfxSimulationIslandAddPairs failed %d\n",ret);
	}

	//E Clear internal flags here for further checking if there're any changes in rigid states and proxies
	{
		pfxOnBroadPhaseFinished(m_broadphaseProxyContainer[0]);
		pfxOnBroadPhaseFinished(m_broadphaseProxyContainer[1]);

		for (unsigned int i = 0; i < m_states.length(); i++) {
			if (!m_states.isRemoved(i)) {
				m_states[i].resetFlags();
			}
		}
	}
}

void PfxRigidBodyWorld::constraintSolver()
{
	unsigned int &numCurrentPairs = m_numPairs[m_pairSwap];
	PfxBroadphasePair *currentPairs = m_pairsBuff[m_pairSwap];

	PfxSolveVelocityAndPositionConstraintsParam param;

	// Rigid bodies
	param.states = m_states.ptr();
	param.bodies = m_bodies.ptr();
	param.numRigidBodies = m_states.length();

	// Contact constraints
	param.contactPairs = currentPairs;
	param.numContactPairs = numCurrentPairs;
	param.contactContainer = &m_contactContainer;

	// Joint constraints
	param.jointPairs = m_jointPairs.ptr();
	param.numJointPairs = m_jointPairs.length();
	param.joints = m_joints.ptr();

	// Solver parameters
	param.velocitySolverIteration[0] = m_velocityIteration[0];
	param.velocitySolverIteration[1] = m_velocityIteration[1];
	param.velocitySolverIteration[2] = m_velocityIteration[2];
	param.positionSolverIteration = m_positionIteration;
	param.timeStep = m_timeStep;
	param.separateWeight = m_separateWeight;
	param.massAmp = 1.6f;

	PfxInt32 ret = pfxSolveVelocityAndPositionConstraints(m_rigidbodyContext, param);
	if (ret != SCE_PFX_OK) SCE_PFX_PRINTF("pfxSolveVelocityAndPositionConstraints failed %d\n", ret);
}

void PfxRigidBodyWorld::sleepOrWakeup()
{
	if(m_simulationFlag & SCE_PFX_ENABLE_SLEEP) {
		PfxSleepOrWakeupParam param;
		param.simulationIsland = &m_simulationIsland;
		param.states = m_states.ptr();
		param.bodies = m_bodies.ptr();
		param.numRigidBodies = m_states.length();
		param.sleepCount = m_sleepCount;
		param.sleepVelocity = m_sleepVelocity;
		
		PfxInt32 ret = pfxSleepOrWakeup(m_rigidbodyContext,param);
		if(ret != SCE_PFX_OK) SCE_PFX_PRINTF("pfxSleepOrWakeup failed %d\n",ret);
	}
}

#define PUSH_PERF(name) {SCE_PFX_PUSH_MARKER(name);SCE_PFX_PUSH_PERF(name);}
#define POP_PERF() {SCE_PFX_POP_PERF();SCE_PFX_POP_MARKER();}
#define SYNC_PERF() {if(m_frame%100==0){SCE_PFX_PRINTF("*** frame %3d ***\n",m_frame);SCE_PFX_PRINT_PERF();}SCE_PFX_SYNC_PERF();}

void PfxRigidBodyWorld::simulate()
{
	PUSH_PERF("simulate");
	pfxRigidBodyContextBeginFrame(m_rigidbodyContext);
	
	PUSH_PERF("applyForce");
	applyForce();
	POP_PERF();
	
	PUSH_PERF("broadphase");
	broadphase();
	POP_PERF();
	
	PUSH_PERF("collision");
	collision();
	POP_PERF();
	
	if(m_simulationFlag & SCE_PFX_ENABLE_CONTACT_CACHE) {
		clearContactCache();
		cacheContacts(0);
	}
	
	PUSH_PERF("solver");
	constraintSolver();
	POP_PERF();
	
	PUSH_PERF("sleepOrWakeup");
	sleepOrWakeup();
	POP_PERF();

	POP_PERF();

	if(m_simulationFlag & SCE_PFX_ENABLE_CONTACT_CACHE) {
		gatherContactCache();
	}

	pfxRigidBodyContextEndFrame(m_rigidbodyContext);

	SYNC_PERF();
	
	m_frame++;
}

void PfxRigidBodyWorld::simulateSubStep(PfxUInt32 n)
{
	if(m_simulationFlag & SCE_PFX_ENABLE_CONTACT_CACHE) {
		clearContactCache();
	}

	for(PfxUInt32 step=0;step<n;step++) {
		applyForce();
		
		broadphase();
		
		collision();
		
		if(m_simulationFlag & SCE_PFX_ENABLE_CONTACT_CACHE) {
			cacheContacts(step);
		}
		
		constraintSolver();
		
		if(m_simulationFlag & SCE_PFX_ENABLE_SLEEP) {
			sleepOrWakeup();
		}
	}
	
	if(m_simulationFlag & SCE_PFX_ENABLE_CONTACT_CACHE) {
		gatherContactCache();
	}

	m_frame++;
}

void PfxRigidBodyWorld::setContactInfo(PfxQueryContactInfo &contactInfo,
	const PfxContactManifold &contact,const PfxRigidState &stateA,const PfxRigidState &stateB)
{
	for(PfxUInt32 i=0;i<contact.getNumContacts();i++) {
		PfxQueryContactPoint &dst = contactInfo.contactPoints[i];
		const PfxContactPoint &src = contact.getContactPoint(i);
		dst.pointA = stateA.getLargePosition() + rotate(stateA.getOrientation(),pfxReadVector3(src.m_localPointA));
		dst.pointB = stateB.getLargePosition() + rotate(stateB.getOrientation(),pfxReadVector3(src.m_localPointB));
		dst.normal = pfxReadVector3(src.m_constraintRow[0].m_normal);
		dst.distance = src.m_distance;
		dst.subDataA = src.m_subDataA;
		dst.subDataB = src.m_subDataB;
	}
	contactInfo.numContactPoints = contact.getNumContacts();
}

void PfxRigidBodyWorld::clearContactCache()
{
	m_numCachedContacts = 0;
	memset(m_cachedContactIdTable,0xffffffff,sizeof(PfxUInt32)*m_states.capacity());
}

void PfxRigidBodyWorld::cacheContacts(PfxUInt32 step)
{
	unsigned int &numCurrentPairs = m_numPairs[m_pairSwap];
	PfxBroadphasePair *currentPairs = m_pairsBuff[m_pairSwap];
	
	if(step == 0) {
		PfxUInt32 j=0;
		for(PfxUInt32 i=0;i<numCurrentPairs;i++) {
			PfxRigidState &stateA = m_states[pfxGetObjectIdA(currentPairs[i])];
			PfxRigidState &stateB = m_states[pfxGetObjectIdB(currentPairs[i])];
			const PfxContactManifold *contact = pfxContactContainerGetContactManifold(m_contactContainer,currentPairs[i]);

			if(contact->getNumContacts() > 0) {
				m_cachedContactPairs[j] = currentPairs[i];
				PfxQueryContactInfo &contactInfo = m_cachedContactInfos[j];
				setContactInfo(contactInfo,*contact,stateA,stateB);
				contactInfo.rigidbodyIdA = pfxGetObjectIdA(currentPairs[i]);
				contactInfo.rigidbodyIdB = pfxGetObjectIdB(currentPairs[i]);
				pfxSetContactId(m_cachedContactPairs[j],j);
				j++;
			}
		}
		m_numCachedContacts = j;
	}
	else {
		PfxUInt32 numOld = m_numCachedContacts;
		PfxUInt32 numNew = numCurrentPairs;
		PfxUInt32 numAdded = 0;
		PfxUInt32 oldId = 0;
		PfxUInt32 newId = 0;
		PfxBroadphasePair *oldPair = m_cachedContactPairs;
		PfxBroadphasePair *newPair = currentPairs;
		PfxBroadphasePair *addPair = m_cachedContactPairs + m_numCachedContacts;
		
		while(oldId<numOld&&newId<numNew) {
			if(pfxGetKey(*newPair) > pfxGetKey(*oldPair)) {
				// skip
				oldPair++;
				oldId++;
			}
			else if(pfxGetKey(*newPair) == pfxGetKey(*oldPair)) {
				// copy
				const PfxContactManifold *contact = pfxContactContainerGetContactManifold(m_contactContainer,*newPair);
				if(contact && contact->getNumContacts() > 0) {
					setContactInfo(
						m_cachedContactInfos[pfxGetContactId(*oldPair)],
						*contact,
						m_states[pfxGetObjectIdA(*newPair)],
						m_states[pfxGetObjectIdB(*newPair)]);
				}

				oldPair++;
				newPair++;
				oldId++;
				newId++;
			}
			else {
				// add
				const PfxContactManifold *contact = pfxContactContainerGetContactManifold(m_contactContainer,*newPair);
				if(contact && contact->getNumContacts() > 0) {
					*addPair = *newPair;
					PfxQueryContactInfo &addContactInfo = m_cachedContactInfos[m_numCachedContacts+numAdded];
					
					setContactInfo(
						addContactInfo,
						*contact,
						m_states[pfxGetObjectIdA(*newPair)],
						m_states[pfxGetObjectIdB(*newPair)]);
					
					addContactInfo.rigidbodyIdA = pfxGetObjectIdA(*newPair);
					addContactInfo.rigidbodyIdB = pfxGetObjectIdB(*newPair);
					
					pfxSetContactId(*addPair,m_numCachedContacts+numAdded);
					numAdded++;
					addPair++;
				}
				
				newPair++;
				newId++;
			}
		};
		
		if(newId<numNew) {
			for(;newId<numNew;newId++,newPair++) {
				// add
				const PfxContactManifold *contact = pfxContactContainerGetContactManifold(m_contactContainer,*newPair);
				if(contact && contact->getNumContacts() > 0) {
					*addPair = *newPair;
					PfxQueryContactInfo &addContactInfo = m_cachedContactInfos[m_numCachedContacts+numAdded];
					
					setContactInfo(
						addContactInfo,
						*contact,
						m_states[pfxGetObjectIdA(*newPair)],
						m_states[pfxGetObjectIdB(*newPair)]);
					
					addContactInfo.rigidbodyIdA = pfxGetObjectIdA(*newPair);
					addContactInfo.rigidbodyIdB = pfxGetObjectIdB(*newPair);
					
					pfxSetContactId(*addPair,m_numCachedContacts+numAdded);
					numAdded++;
					addPair++;
				}
			}
		}
		
		m_numCachedContacts += numAdded;
	}

	void *workBuff = m_pool.allocate(sizeof(PfxBroadphasePair)*m_numCachedContacts);
	pfxSort(m_cachedContactPairs,(PfxBroadphasePair*)workBuff,m_numCachedContacts);
	m_pool.deallocate(workBuff);
}

void PfxRigidBodyWorld::gatherContactCache()
{
	for(PfxUInt32 i=0;i<m_numCachedContacts;i++) {
		PfxBroadphasePair &pair = m_cachedContactPairs[i];
		PfxUInt32 idA = pfxGetObjectIdA(pair);
		PfxUInt32 idB = pfxGetObjectIdB(pair);
		m_cachedContactInfos[pfxGetContactId(pair)].pairId = i;
		
		pair.set32(0,0xffffffff); // A-Next
		pair.set32(2,0xffffffff); // B-Next(Sort key isn't needed anymore.)
		
		// Connect to A
		if(m_cachedContactIdTable[idA] == 0xffffffff) {
			m_cachedContactIdTable[idA] = i;
		}
		else {
			PfxUInt32 tmpId = m_cachedContactIdTable[idA];
			m_cachedContactIdTable[idA] = i;
			pair.set32(0,tmpId);
		}
		
		// Connect to B
		if(m_cachedContactIdTable[idB] == 0xffffffff) {
			m_cachedContactIdTable[idB] = i;
		}
		else {
			PfxUInt32 tmpId = m_cachedContactIdTable[idB];
			m_cachedContactIdTable[idB] = i;
			pair.set32(2,tmpId);
		}
	}
	
	// check cached contact
	/*
	SCE_PFX_PRINTF("----- pairs (original) -----\n");
	unsigned int &numCurrentPairs = m_numPairs[m_pairSwap];
	PfxBroadphasePair *currentPairs = m_pairsBuff[m_pairSwap];
	for(PfxUInt32 i=0;i<numCurrentPairs;i++) {
		PfxBroadphasePair &pair = currentPairs[i];
		const PfxContactManifold *contact = pfxContactContainerGetContactManifold(m_contactContainer,pair);
		SCE_PFX_PRINTF("%3u A:%3u B:%3u Contacts:%u\n",i,pfxGetObjectIdA(pair),pfxGetObjectIdB(pair),contact->getNumContacts());
	}

	SCE_PFX_PRINTF("----- pairs -----\n");
	for(PfxUInt32 i=0;i<m_numCachedContacts;i++) {
		PfxBroadphasePair &pair = m_cachedContactPairs[i];
		PfxQueryContactInfo &contactInfo = m_cachedContactInfos[pfxGetContactId(pair)];
		SCE_PFX_PRINTF("%3u A:%3u B:%3u Contacts:%u\n",i,pfxGetObjectIdA(pair),pfxGetObjectIdB(pair),contactInfo.numContactPoints);
	}

	SCE_PFX_PRINTF("----- query -----\n");
	for(PfxUInt32 i=0;i<m_states.size();i++) {
		PfxUInt32 pid = m_cachedContactIdTable[i];
		SCE_PFX_PRINTF("%3u\n",i);
		while(pid != 0xffffffff) {
			PfxBroadphasePair &pair = m_cachedContactPairs[pid];
			PfxQueryContactInfo &contactInfo = m_cachedContactInfos[pfxGetContactId(pair)];

			SCE_PFX_PRINTF("   Pair:%u(Contacts:%u)\n",pid,contactInfo.numContactPoints);

			if(pfxGetObjectIdA(pair) == i) {
				pid = pair.get32(0);
			}
			else {
				pid = pair.get32(2);
			}
		}
	}
	*/
}

const PfxQueryContactInfo *PfxRigidBodyWorld::queryContact(PfxUInt32 rigidBodyIdA,PfxUInt32 rigidBodyIdB) const
{
	if(rigidBodyIdA >= m_states.capacity() || rigidBodyIdB >= m_states.capacity())
		return NULL;
	
	PfxUInt32 pid = m_cachedContactIdTable[rigidBodyIdA];
	
	while(pid != 0xffffffff) {
		PfxBroadphasePair &pair = m_cachedContactPairs[pid];
		PfxUInt32 idA = pfxGetObjectIdA(pair);
		PfxUInt32 idB = pfxGetObjectIdB(pair);
		
		if((idA == rigidBodyIdA && idB == rigidBodyIdB) || (idA == rigidBodyIdB && idB == rigidBodyIdA)) {
			return m_cachedContactInfos + pfxGetContactId(pair);
		}
		
		if(idA == rigidBodyIdA) {
			pid = pair.get32(0);
		}
		else {
			pid = pair.get32(2);
		}
	}
	
	return NULL;
}

const PfxQueryContactInfo *PfxRigidBodyWorld::queryFirstContact(PfxUInt32 rigidBodyId) const
{
	if(rigidBodyId >= m_states.capacity())
		return NULL;
	
	PfxUInt32 pid = m_cachedContactIdTable[rigidBodyId];
	
	if(pid != 0xffffffff) {
		return m_cachedContactInfos + pfxGetContactId(m_cachedContactPairs[pid]);
	}
	
	return NULL;
}

const PfxQueryContactInfo *PfxRigidBodyWorld::queryNextContact(const PfxQueryContactInfo *contactInfo,PfxUInt32 rigidBodyId) const
{
	if(rigidBodyId >= m_states.capacity() || !contactInfo)
		return NULL;
	
	PfxUInt32 pid = contactInfo->pairId;
	PfxBroadphasePair &pair = m_cachedContactPairs[pid];

	if(pfxGetObjectIdA(pair) == rigidBodyId) {
		pid = pair.get32(0);
	}
	else {
		pid = pair.get32(2);
	}
	
	if(pid != 0xffffffff) {
		return m_cachedContactInfos + pfxGetContactId(m_cachedContactPairs[pid]);
	}
	
	return NULL;
}

PfxBool PfxRigidBodyWorld::checkNonContactPair(PfxUInt32 rigidBodyIdA,PfxUInt32 rigidBodyIdB)
{
	if(m_numNonContactPairs == 0) return false;

	PfxUInt32 key = pfxCreateUniqueKey(SCE_PFX_MIN(rigidBodyIdA,rigidBodyIdB),SCE_PFX_MAX(rigidBodyIdA,rigidBodyIdB));

	int left = 0;
	int right = (int)m_numNonContactPairs-1;
	int mid;

	while(left < right) {
		mid = (left + right) / 2;
		if(m_nonContactPairs[mid] == key) {
			return true;
		}
		else if(m_nonContactPairs[mid] < key) {
			left = mid + 1;
		}
		else { // m_nonContactPairs[mid] > key
			right = mid - 1;
		}
	}

	return false;
}

void PfxRigidBodyWorld::appendNonContactPair(PfxUInt32 rigidBodyIdA,PfxUInt32 rigidBodyIdB)
{
	SCE_PFX_ALWAYS_ASSERT(m_numNonContactPairs < m_maxNonContactPairs);

	PfxUInt32 key = pfxCreateUniqueKey(SCE_PFX_MIN(rigidBodyIdA,rigidBodyIdB),SCE_PFX_MAX(rigidBodyIdA,rigidBodyIdB));

	if(m_numNonContactPairs == 0) {
		m_nonContactPairs[m_numNonContactPairs++] = key;
		return;
	}

	int left = 0;
	int right = (int)m_numNonContactPairs-1;
	int mid;

	while(left <= right) {
		mid = (left + right) / 2;
		if(m_nonContactPairs[mid] == key) {
			return;
		}
		else if(m_nonContactPairs[mid] < key) {
			left = mid + 1;
		}
		else { // m_nonContactPairs[mid] > key
			right = mid - 1;
		}
	}

	m_numNonContactPairs++;

	for(int i=left;i<(int)m_numNonContactPairs;i++) {
		PfxUInt32 tmp = m_nonContactPairs[i];
		m_nonContactPairs[i] = key;
		key = tmp;
	}
}

void PfxRigidBodyWorld::removeNonContactPair(PfxUInt32 rigidBodyIdA,PfxUInt32 rigidBodyIdB)
{
	if(m_numNonContactPairs == 0) return;

	PfxUInt32 key = pfxCreateUniqueKey(SCE_PFX_MIN(rigidBodyIdA,rigidBodyIdB),SCE_PFX_MAX(rigidBodyIdA,rigidBodyIdB));

	int left = 0;
	int right = (int)m_numNonContactPairs-1;
	int mid;

	while(left <= right) {
		mid = (left + right) / 2;
		if(m_nonContactPairs[mid] == key) {
			for(int i=mid;i<(int)m_numNonContactPairs-1;i++) {
				m_nonContactPairs[i] = m_nonContactPairs[i+1];
			}
			m_numNonContactPairs--;
			
			return;
		}
		else if(m_nonContactPairs[mid] < key) {
			left = mid + 1;
		}
		else { // m_nonContactPairs[mid] > key
			right = mid - 1;
		}
	}

	return;
}

void PfxRigidBodyWorld::setCastArea(const PfxLargePosition &areaMin,const PfxLargePosition &areaMax)
{
	m_areaMin = areaMin;
	m_areaMax = areaMax;
}

void PfxRigidBodyWorld::castSingleRay(const PfxRayInput &rayIn,PfxRayOutput &rayOut)
{
	PfxRayCastParam param;
	param.states = m_states.ptr();
	param.collidables = m_collidables.ptr();
	param.rangeMin = m_areaMin;
	param.rangeMax = m_areaMax;

	PfxRayOutput out[2];

	param.proxyContainer = &m_broadphaseProxyContainer[0];
	pfxCastSingleRay(rayIn,out[0],param);

	param.proxyContainer = &m_broadphaseProxyContainer[1];
	pfxCastSingleRay(rayIn,out[1],param);
	
	if(out[1].m_contactFlag && out[1].m_variable < out[0].m_variable) {
		rayOut = out[1];
	}
	else {
		rayOut = out[0];
	}
}

void PfxRigidBodyWorld::castRays(PfxRayInput *rayInputs,PfxRayOutput *rayOutputs,int numRays)
{
	PfxRayCastParam param;
	param.states = m_states.ptr();
	param.collidables = m_collidables.ptr();
	param.rangeMin = m_worldMin;
	param.rangeMax = m_worldMax;

	for(int i=0;i<numRays;i++) {
		PfxRayOutput out[2];

		param.proxyContainer = &m_broadphaseProxyContainer[0];
		pfxCastSingleRay(rayInputs[i],out[0],param);

		param.proxyContainer = &m_broadphaseProxyContainer[1];
		pfxCastSingleRay(rayInputs[i],out[1],param);

		if(out[1].m_contactFlag && out[1].m_variable < out[0].m_variable) {
			rayOutputs[i] = out[1];
		}
		else {
			rayOutputs[i] = out[0];
		}
	}
}

void PfxRigidBodyWorld::castSingleSphere(const PfxSphereInput &sphereIn, PfxSphereOutput &sphereOut)
{
	PfxSphereCastParam param;
	param.states = m_states.ptr();
	param.collidables = m_collidables.ptr();
	param.rangeMin = m_areaMin;
	param.rangeMax = m_areaMax;

	PfxSphereOutput out[2];

	param.proxyContainer = &m_broadphaseProxyContainer[0];
	pfxCastSingleSphere(sphereIn,out[0],param);

	param.proxyContainer = &m_broadphaseProxyContainer[1];
	pfxCastSingleSphere(sphereIn,out[1],param);
	
	if(out[1].m_contactFlag && out[1].m_variable < out[0].m_variable) {
		sphereOut = out[1];
	}
	else {
		sphereOut = out[0];
	}
}

void PfxRigidBodyWorld::castSpheres(PfxSphereInput *sphereInputs, PfxSphereOutput *sphereOutputs, int numSpheres)
{
	PfxSphereCastParam param;
	param.states = m_states.ptr();
	param.collidables = m_collidables.ptr();
	param.rangeMin = m_worldMin;
	param.rangeMax = m_worldMax;

	for(int i=0;i<numSpheres;i++) {
		PfxSphereOutput out[2];

		param.proxyContainer = &m_broadphaseProxyContainer[0];
		pfxCastSingleSphere(sphereInputs[i],out[0],param);

		param.proxyContainer = &m_broadphaseProxyContainer[1];
		pfxCastSingleSphere(sphereInputs[i],out[1],param);

		if(out[1].m_contactFlag && out[1].m_variable < out[0].m_variable) {
			sphereOutputs[i] = out[1];
		}
		else {
			sphereOutputs[i] = out[0];
		}
	}
}

void PfxRigidBodyWorld::castSingleCapsule(const PfxCapsuleInput &capsuleIn, PfxCapsuleOutput &capsuleOut)
{
	PfxCapsuleCastParam param;
	param.states = m_states.ptr();
	param.collidables = m_collidables.ptr();
	param.rangeMin = m_areaMin;
	param.rangeMax = m_areaMax;

	PfxCapsuleOutput out[2];

	param.proxyContainer = &m_broadphaseProxyContainer[0];
	pfxCastSingleCapsule(capsuleIn,out[0],param);

	param.proxyContainer = &m_broadphaseProxyContainer[1];
	pfxCastSingleCapsule(capsuleIn,out[1],param);
	
	if(out[1].m_contactFlag && out[1].m_variable < out[0].m_variable) {
		capsuleOut = out[1];
	}
	else {
		capsuleOut = out[0];
	}
}

void PfxRigidBodyWorld::castCapsules(PfxCapsuleInput *capsuleInputs, PfxCapsuleOutput *capsuleOutputs, int numCapsules)
{
	PfxCapsuleCastParam param;
	param.states = m_states.ptr();
	param.collidables = m_collidables.ptr();
	param.rangeMin = m_worldMin;
	param.rangeMax = m_worldMax;

	for (int i = 0; i<numCapsules; i++) {
		PfxCapsuleOutput out[2];

		param.proxyContainer = &m_broadphaseProxyContainer[0];
		pfxCastSingleCapsule(capsuleInputs[i], out[0], param);

		param.proxyContainer = &m_broadphaseProxyContainer[1];
		pfxCastSingleCapsule(capsuleInputs[i], out[1], param);

		if(out[1].m_contactFlag && out[1].m_variable < out[0].m_variable) {
			capsuleOutputs[i] = out[1];
		}
		else {
			capsuleOutputs[i] = out[0];
		}
	}
}

} // namespace PhysicsEffects
} // namespace sce
