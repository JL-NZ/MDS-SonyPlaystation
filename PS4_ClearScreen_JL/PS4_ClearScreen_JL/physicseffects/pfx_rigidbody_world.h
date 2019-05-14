/* SIE CONFIDENTIAL
 * PlayStation(R)4 Programmer Tool Runtime Library Release 05.008.001
 *                Copyright (C) 2016 Sony Interactive Entertainment Inc.
 *                                                All Rights Reserved.
 */

#ifndef _SCE_PFX_RIGIDBODY_WORLD_H
#define _SCE_PFX_RIGIDBODY_WORLD_H

#include "physics_effects.h"

namespace sce{
namespace PhysicsEffects{

///////////////////////////////////////////////////////////////////////////////
// Simulation flag

#define SCE_PFX_ENABLE_SLEEP					0x01
#define SCE_PFX_ENABLE_CONTACT_CACHE			0x02

///////////////////////////////////////////////////////////////////////////////
// PfxRigidBodyWorldParam

/// @~English
/// @brief Initialization parameters for the rigid body world
/// @details Initialization parameters for the rigid body world.
/// @~Japanese
/// @brief 剛体ワールド初期化パラメータ
/// @details 剛体ワールド初期化時に必要なパラメータです。
struct PfxRigidBodyWorldParam {
	// Pool memory
	void *poolBuff;					///< @brief Pool buffer
	PfxUInt32 poolBytes;			///< @brief Bytes of the pool buffer
	
	// Work buffer size
	PfxUInt32 contextWorkBytes;		///< @brief Size of the temporal buffer
	PfxUInt32 ccdWorkBytes;			///< @brief Size of the additional buffer for CCD
	
	// Number of objects
	PfxUInt32 maxRigidBodies;		///< @brief Maximum number of the rigid bodies
	PfxUInt32 maxJoints;			///< @brief Maximum number of the joints
	PfxUInt32 maxShapes;			///< @brief Maximum number of the shapes
	PfxUInt32 maxContacts;			///< @brief Maximum number of the contacts
	PfxUInt32 maxNonContactPairs;	///< @brief Maximum number of non contact pairs

	// World config
	PfxInt32 segmentWidth;
	PfxLargePosition worldMin;		///< @brief Minimum position of the world's AABB
	PfxLargePosition worldMax;		///< @brief Maximum position of the world's AABB
	PfxVector3 gravity;				///< @brief Gravity

	// Simulation config
	PfxFloat timeStep;				///< @brief Time step
	PfxFloat separateWeight;		///< @brief Weight factor to fix position correction
	PfxUInt32 velocityIteration[3];	///< @brief Loop count for solver iteration
	PfxUInt32 positionIteration;	///< @brief Loop count for solver iteration
	PfxUInt32 sleepCount;			///< @brief Sleep count
	PfxFloat sleepVelocity;			///< @brief Sleep velocity

	PfxUInt32 multiThreadFlag;		///< @brief Multithread flag
	PfxUInt32 numWorkerThreads;		///< @brief Number of tasks / threads

	PfxUInt32 simulationFlag;		///< @brief Simulation flag
	
#ifdef __psp2__
	void *userJobManagerSequenceFactoryInterface; ///< @brief sequence fatory interface of user job manager
#else
	sce::Job::SequenceFactoryInterface *userJobManagerSequenceFactoryInterface; ///< @brief sequence fatory interface of user job manager
#endif

	SCE_PFX_EXCLUDE_DOC(
		PfxRigidBodyWorldParam() {
			contextWorkBytes = 1*1024*1024;
			ccdWorkBytes = 0;
			
			maxRigidBodies = 500;
			maxJoints = 500;
			maxShapes = 100;
			maxContacts = 4000;
			maxNonContactPairs = 100;
			
			segmentWidth = 0;
			worldMin = PfxLargePosition(PfxVector3(-250.0f));
			worldMax = PfxLargePosition(PfxVector3( 250.0f));
			gravity = PfxVector3(0.0f,-9.8f,0.0f);

			timeStep = 0.0166666666f;
			separateWeight = 0.8f;
			velocityIteration[0] = 3;
			velocityIteration[1] = 3;
			velocityIteration[2] = 3;
			positionIteration = 2;

			sleepCount = 180;
			sleepVelocity = 0.1f;

			poolBuff = NULL;
			poolBytes = 0;

			multiThreadFlag = 0;
			numWorkerThreads = 0; // 0,1 = single thread

			simulationFlag = 0;

			userJobManagerSequenceFactoryInterface = NULL;
		}
	)
};

///////////////////////////////////////////////////////////////////////////////
// PfxQueryContactInfo

/// @~English
/// @brief Contact point
/// @details This structure stores a contact point result from a contact query.
/// @~Japanese
/// @brief 衝突点
/// @details コンタクト問い合わせ結果の衝突点を格納します。
struct PfxQueryContactPoint {
	PfxLargePosition pointA;				///< @brief Contact point A in the world coordinates
	PfxLargePosition pointB;				///< @brief Contact point B in the world coordinates
	PfxVector3 normal;						///< @brief Normal vector
	PfxSubData subDataA;					///< @brief Sub data A
	PfxSubData subDataB;					///< @brief Sub data B
	PfxFloat distance;						///< @brief Distance
};

/// @~English
/// @brief Contact information
/// @details This structure stores a contact manifold result from a contact query.
/// @~Japanese
/// @brief 衝突情報
/// @details コンタクト問い合わせ結果の衝突情報を格納します。
struct PfxQueryContactInfo {
	PfxQueryContactPoint contactPoints[4];	///< @brief Contact points
	PfxUInt32 numContactPoints;				///< @brief Number of contact points
	PfxUInt16 rigidbodyIdA;					///< @brief Rigid body A ID
	PfxUInt16 rigidbodyIdB;					///< @brief Rigid body B ID
	PfxUInt32 pairId;						///< @brief Pair ID
};

///////////////////////////////////////////////////////////////////////////////
// PfxRigidBodyWorld class

/// @~English
/// @brief Rigid body world
/// @details This class represents a base of a physics engine combined with low level APIs.
/// @~Japanese
/// @brief 剛体ワールド
/// @details ローレベルAPIの組み合わせによる、物理エンジンのひな型クラスです。
class PfxRigidBodyWorld
{
	friend void serializeInitRead(PfxSerializeCapacity *capacity,PfxSerializeBuffer *buffer);
	friend void serializeTermRead(PfxSerializeCapacity *capacity,PfxSerializeBuffer *buffer);
	friend void serializeInitReadDiff(PfxSerializeCapacity *capacity,PfxSerializeBuffer *buffer);
	friend void serializeTermReadDiff(PfxSerializeCapacity *capacity,PfxSerializeBuffer *buffer);
	friend void serializeInitWrite(PfxSerializeCapacity *capacity,PfxSerializeBuffer *buffer);
	friend void serializeTermWrite(PfxSerializeCapacity *capacity,PfxSerializeBuffer *buffer);
	friend PfxBool serializeResize(PfxSerializeCapacity *capacity,PfxSerializeBuffer *buffer,const PfxSerializeCapacity *capacityLoaded);
	friend void serializeError(PfxInt32 err,PfxUInt64 tagId);

protected:
	//J プールメモリ
	//E Pool memory
	PfxHeapManager m_pool;

	//J ワールド
	//E World
	PfxInt32 m_segmentWidth;
	PfxLargePosition m_worldMin;
	PfxLargePosition m_worldMax;
	PfxVector3 m_gravity;

	//J 剛体
	//E Rigid body
	PfxPoolArray<PfxRigidState> m_states;
	PfxPoolArray<PfxRigidBody>  m_bodies;
	PfxPoolArray<PfxCollidable> m_collidables;

	//J 形状
	//E Shape
	PfxPoolArray<PfxShape> m_shapes;

	//J ジョイント
	//E Joint
	PfxPoolArray<PfxConstraintPair> m_jointPairs;
	PfxPoolArray<PfxJoint> m_joints;

	//J ノンコンタクトペア
	//E Non contact pairs
	PfxUInt32 m_numNonContactPairs;
	const PfxUInt32 m_maxNonContactPairs;
	PfxUInt32 *m_nonContactPairs;

	//J キャッシュされるペアバッファ
	//E Cached pair buffers
	PfxUInt32 m_numPairs[2];
	const PfxUInt32 m_maxPairs;
	PfxBroadphasePair *m_pairsBuff[2];

	//J 剛体シミュレーションコンテキスト
	//E Rigid body simulation context
	PfxRigidBodyContext m_rigidbodyContext;

	//J ブロードフェーズコンテナ
	//E Broadphase container 
	PfxBroadphaseProxyContainer m_broadphaseProxyContainer[2];

	//J コンタクトコンテナ
	//E Contact container
	PfxContactContainer m_contactContainer;

	//J シミュレーションアイランド
	//E Simulation island
	PfxSimulationIsland m_simulationIsland;

	PfxFloat m_timeStep;
	PfxFloat m_separateWeight;
	PfxUInt32 m_velocityIteration[3];
	PfxUInt32 m_positionIteration;
	PfxUInt32 m_sleepCount;
	PfxFloat m_sleepVelocity;
	PfxUInt32 m_simulationFlag;

	PfxUInt32 m_numWorkerThreads;
	PfxUInt32 m_multiThreadFlag;
	
#ifdef __psp2__
	void *m_userJobManagerSequenceFactoryInterface;
#else
	sce::Job::SequenceFactoryInterface *m_userJobManagerSequenceFactoryInterface;
#endif

	//E Context Buffer
	//J コンテキストバッファ
	PfxUInt32 m_contextWorkBytes;
	void *m_rigidbodyContextBuff;
	
	//J 内部使用パラメータ
	//E Parameters for internal use
	PfxUInt32 m_pairSwap;
	PfxUInt32 m_frame;

	PfxLargePosition m_areaMin;
	PfxLargePosition m_areaMax;
	
	//J クエリ用コンタクトキャッシュ
	//E Contact cache for query
	PfxUInt32 m_numCachedContacts;
	PfxUInt32 *m_cachedContactIdTable;
	PfxQueryContactInfo *m_cachedContactInfos;
	PfxBroadphasePair *m_cachedContactPairs;
	
	virtual void setContactInfo(PfxQueryContactInfo &contactInfo,
		const PfxContactManifold &contact,const PfxRigidState &stateA,const PfxRigidState &stateB);
	void clearContactCache();
	void cacheContacts(PfxUInt32 step);
	void gatherContactCache();
	
	void printPairs(const char *msg,PfxBroadphasePair *pairs,int n);
	
public:
	void applyForce();

	void broadphase();

	void collision();

	void constraintSolver();

	void sleepOrWakeup();

	void updateProxies();
	
	void reconstructRigidBodyContext(PfxInt32 segmentWidth,PfxUInt32 numWorkerThreads,PfxUInt32 multiThreadFlag);
public:
	///
	/// @~English
	/// @brief Query bytes of the work buffer needed for the rigid body world.
	/// @details Get the size of bytes which should be given to PfxRigidBodyWorldParam::poolBuff.
	/// @param param Initialization parameter of the rigid body world
	/// @return Returns bytes of the buffer
	/// @~Japanese
	/// @brief 剛体ワールドに必要なワークバッファのサイズを取得する。
	/// @details PfxRigidBodyWorldParam::poolBuffにセットすべきバッファのサイズを取得する。
	/// @param param 剛体ワールドの初期化パラメータ
	/// @return バッファサイズを返す。
	static PfxUInt32 getRigidBodyWorldBytes(const PfxRigidBodyWorldParam &param);

	///
	/// @~English
	/// @brief Constractor of the rigid body world
	/// @details Create the rigid body world.
	/// @param param Initialization parameter of the rigid body world
	/// @~Japanese
	/// @brief 剛体ワールドのコンストラクタ
	/// @details 剛体ワールドを作成します。
	/// @param param 剛体ワールドの初期化パラメータ
	PfxRigidBodyWorld(const PfxRigidBodyWorldParam &param);
	
	///
	/// @~English
	/// @brief Destructor of the rigid body world
	/// @details Destroy the rigid body world.
	/// @~Japanese
	/// @brief 剛体ワールドのデストラクタ
	/// @details 剛体ワールドを破棄します。
	virtual ~PfxRigidBodyWorld();
	
	///
	/// @~English
	/// @brief Initialize the rigid body world
	/// @details Initialize the rigid body world.
	/// @~Japanese
	/// @brief 剛体ワールドの初期化
	/// @details 剛体ワールドを初期化します。
	void initialize();
	
	///
	/// @~English
	/// @brief Finalize the rigid body world
	/// @details Finalize the rigid body world.
	/// @~Japanese
	/// @brief 剛体ワールドの終了処理
	/// @details 剛体ワールドを終了します。
	void finalize();
	
	///
	/// @~English
	/// @brief Reset the rigid body world
	/// @details Reset the rigid body world to the initial state.
	/// @~Japanese
	/// @brief 剛体ワールドのリセット
	/// @details 剛体ワールドを初期状態に戻します。
	void reset();
	
	///
	/// @~English
	/// @brief Simulate the world
	/// @details Advance the world's time by the time step.
	/// @~Japanese
	/// @brief シミュレーションを実行する。
	/// @details ワールドの時間をタイムステップ分、進めます。
	void simulate();
	
	///
	/// @~English
	/// @brief Simulate the world multiple times
	/// @details Simulate the world multiple times.
	/// @param n Execution count
	/// @~Japanese
	/// @brief シミュレーションの複数実行
	/// @details シミュレーションを複数回、実行します。
	/// @param n 実行回数
	void simulateSubStep(PfxUInt32 n);

	/// @name Ray casting
	/// @{
	
	///
	/// @~English
	/// @brief Specify the range of ray casting
	/// @details Set the default world size if not specified.
	/// @param areaMin Minimum position of the range
	/// @param areaMax Maximum position of the range
	/// @~Japanese
	/// @brief レイをキャストする範囲を指定する
	///	@details 指定が無い場合は、デフォルトのワールドサイズが使用されます。
	/// @param areaMin 範囲の最小座標
	/// @param areaMax 範囲の最大座標
	void setCastArea(const PfxLargePosition &areaMin,const PfxLargePosition &areaMax);
	
	///
	/// @~English
	/// @brief Cast a single ray
	/// @details Detect an intersection point closest to the start point of the ray.
	/// @param rayIn Ray
	/// @param[out] rayOut Result of ray casting
	/// @~Japanese
	/// @brief 単一のレイをキャストする
	/// @details 始点から最も近い交差点を検出します。
	/// @param rayIn レイ
	/// @param[out] rayOut レイキャスト結果
	void castSingleRay(const PfxRayInput &rayIn,PfxRayOutput &rayOut);

	void castSingleSphere(const PfxSphereInput &sphereIn, PfxSphereOutput &sphereOut);

	void castSingleCapsule(const PfxCapsuleInput &capsuleIn, PfxCapsuleOutput &capsuleOut);

	/// 
	/// @~English
	/// @brief Cast multiple rays
	/// @details Cast multiple rays.
	/// @param rayInputs Array of rays
	/// @param[out] rayOutputs Array of hits
	/// @param numRays Number of rays
	/// @~Japanese
	/// @brief 複数レイのキャスト
	/// @details 複数のレイをキャストします。
	/// @param rayInputs レイの配列
	/// @param[out] rayOutputs ヒット結果の配列
	/// @param numRays レイの数
	void castRays(PfxRayInput *rayInputs,PfxRayOutput *rayOutputs,int numRays);
	
	void castSpheres(PfxSphereInput *sphereInputs, PfxSphereOutput *sphereOutputs, int numSpheres);

	void castCapsules(PfxCapsuleInput *capsuleInputs, PfxCapsuleOutput *capsuleOutputs, int numCapsules);
	
	/// @}
	
	/// @name Non contact pair
	/// @{
	
	///
	/// @~English
	/// @brief Append a non contact pair
	/// @details Append a non contact pair.
	/// @param rigidBodyIdA Rigid body A ID
	/// @param rigidBodyIdB Rigid body B ID
	/// @~Japanese
	/// @brief 非衝突ペアを追加する
	/// @details 非衝突ペアを追加します。
	/// @param rigidBodyIdA 剛体AのID
	/// @param rigidBodyIdB 剛体BのID
	void appendNonContactPair(PfxUInt32 rigidBodyIdA,PfxUInt32 rigidBodyIdB);

	///
	/// @~English
	/// @brief Remove a non contact pair
	/// @details Remove a non contact pair.
	/// @param rigidBodyIdA Rigid body A ID
	/// @param rigidBodyIdB Rigid body B ID
	/// @~Japanese
	/// @brief 非衝突ペアを削除する
	/// @details 非衝突ペアを削除します。
	/// @param rigidBodyIdA 剛体AのID
	/// @param rigidBodyIdB 剛体BのID
	void removeNonContactPair(PfxUInt32 rigidBodyIdA,PfxUInt32 rigidBodyIdB);

	///
	/// @~English
	/// @brief Check if two rigid bodies collide
	/// @details Check if two rigid bodies collide.
	/// @param rigidBodyIdA Rigid body A ID
	/// @param rigidBodyIdB Rigid body B ID
	/// @return Returns true if two rigid bodies don't collide.
	/// @~Japanese
	/// @brief ２つの剛体が衝突するかどうかをチェックする
	/// @details ２つの剛体が衝突するかどうかをチェックします。
	/// @param rigidBodyIdA 剛体AのID
	/// @param rigidBodyIdB 剛体BのID
	/// @return 非衝突ペアであればtrueを返す。
	PfxBool checkNonContactPair(PfxUInt32 rigidBodyIdA,PfxUInt32 rigidBodyIdB);
	
	/// @}
	
	/// @name Contact query
	/// @{
	
	///
	/// @~English
	/// @brief Query contact of two rigid bodies
	/// @details  Set SCE_PFX_ENABLE_CONTACT_CACHE to PfxRigidBodyWorldParam::simulationFlag
	/// and initialize the world to create the contact buffer for query.
	/// @param rigidBodyIdA Rigid body A ID
	/// @param rigidBodyIdB Rigid body B ID
	/// @return Returns contact query. Returns NULL if there is no collision.
	/// @~Japanese
	/// @brief ２つの剛体の衝突を問い合わせる
	/// @details クエリ用のコンタクトキャッシュバッファを確保するため、
	/// PfxRigidBodyWorldParam::simulationFlagにSCE_PFX_ENABLE_CONTACT_CACHE
	/// をセットしてワールドを初期化してください。
	/// @param rigidBodyIdA 剛体AのID
	/// @param rigidBodyIdB 剛体BのID
	/// @return 衝突クエリを返す。衝突していない場合はNULLを返す。
	const PfxQueryContactInfo *queryContact(PfxUInt32 rigidBodyIdA,PfxUInt32 rigidBodyIdB) const;

	///
	/// @~English
	/// @brief Query the first contact
	/// @details Get contacts related to the specified rigid body in sequence.
	/// @param rigidBodyId Rigid body ID
	/// @return Returns first contact query. Returns NULL if there is no collision.
	/// @~Japanese
	/// @brief 最初の衝突を問い合わせる
	/// @details 指定された剛体の関連する衝突を連続して取得します。
	/// @param rigidBodyId 剛体ID
	/// @return 最初の衝突クエリを返す。衝突していない場合はNULLを返す。
	const PfxQueryContactInfo *queryFirstContact(PfxUInt32 rigidBodyId) const;

	///
	/// @~English
	/// @brief Query the next contact
	/// @details Get contacts related to the specified rigid body in sequence.
	/// @param contactInfo contact query
	/// @param rigidBodyId Rigid body ID
	/// @return Returns next contact query. Returns NULL if there is no collision.
	/// @~Japanese
	/// @brief 次の衝突を問い合わせる
	/// @details 指定された剛体の関連する衝突を連続して取得します。
	/// @param contactInfo 衝突クエリ
	/// @param rigidBodyId 剛体ID
	/// @return 次の衝突クエリを返す。もし無ければNULLを返す。
	const PfxQueryContactInfo *queryNextContact(const PfxQueryContactInfo *contactInfo,PfxUInt32 rigidBodyId) const;

	/// @}

	/// @name Context / Containers
	/// @{
	
	/// 
	/// @~English
	/// @brief Get the rigid body context
	/// @details Get the rigid body context.
	/// @return Returns the rigid body context.
	/// @~Japanese
	/// @brief 剛体コンテキストの取得
	/// @details 剛体コンテキストを取得します。
	/// @return 剛体コンテキストを返します。
	const PfxRigidBodyContext &getRigidBodyContext() const {return m_rigidbodyContext;}

	/// 
	/// @~English
	/// @brief Get the broadphase proxy container for active objects
	/// @details Get the broadphase proxy container for active objects.
	/// @return Returns the broadphase proxy container for active objects.
	/// @~Japanese
	/// @brief アクティブ剛体のブロードフェーズプロキシコンテナの取得
	/// @details アクティブ剛体のブロードフェーズプロキシコンテナを取得します。
	/// @return アクティブ剛体のブロードフェーズプロキシコンテナを返します。
	const PfxBroadphaseProxyContainer &getActiveBroadphaseProxyContainer() const {return m_broadphaseProxyContainer[1];}

	/// 
	/// @~English
	/// @brief Get the broadphase proxy container for static objects
	/// @details Get the broadphase proxy container for static objects.
	/// @return Returns the broadphase proxy container for static objects.
	/// @~Japanese
	/// @brief 静止剛体のブロードフェーズプロキシコンテナの取得
	/// @details 静止剛体のブロードフェーズプロキシコンテナを取得します。
	/// @return 静止剛体のブロードフェーズプロキシコンテナを返します。
	const PfxBroadphaseProxyContainer &getStaticBroadphaseProxyContainer() const {return m_broadphaseProxyContainer[0];}

	/// 
	/// @~English
	/// @brief Get the contact container
	/// @details Get the contact container.
	/// @return Returns the contact container.
	/// @~Japanese
	/// @brief コンタクトコンテナの取得
	/// @details コンタクトコンテナを取得します。
	/// @return コンタクトコンテナを返します。
	const PfxContactContainer &getContactContainer() const {return m_contactContainer;}

	/// 
	/// @~English
	/// @brief Get the simulation island container
	/// @details Get the simulation island container.
	/// @return Returns the simulation island container.
	/// @~Japanese
	/// @brief シミュレーションアイランドコンテナの取得
	/// @details シミュレーションアイランドコンテナを取得します。
	/// @return シミュレーションアイランドコンテナを返します。
	const PfxSimulationIsland &getSimulationIsland() const {return m_simulationIsland;}
	
	/// @}

	/// @name World operation
	/// @{
	
	///
	/// @~English
	/// @brief Get world minimum position
	/// @details Get the minimum position of the world.
	/// @return Returns the minimum position of the world.
	/// @~Japanese
	/// @brief ワールドの最小座標値の取得
	/// @details ワールドの最小座標値を取得します。
	/// @return ワールドの最小座標値を返します。
	const PfxLargePosition &getWorldMin() const {return m_worldMin;}

	///
	/// @~English
	/// @brief Get world maximum position
	/// @details Get the maximum position of the world.
	/// @return Returns the maximum position of the world.
	/// @~Japanese
	/// @brief ワールドの最大座標値の取得
	/// @details ワールドの最大座標値を取得します。
	/// @return ワールドの最大座標値を返します。
	const PfxLargePosition &getWorldMax() const {return m_worldMax;}
	
	///
	/// @~English
	/// @brief Get gravity
	/// @details Get gravity.
	/// @return Returns gravity.
	/// @~Japanese
	/// @brief 重力の取得
	/// @details 重力を取得します。
	/// @return 重力を返します。
	const PfxVector3 &getGravity() const {return m_gravity;}

	///
	/// @~English
	/// @brief Get sleep threshold
	/// @details Get sleep threshold.
	/// @return Returns sleep threshold.
	/// @~Japanese
	/// @brief スリープの閾値の取得
	/// @details スリープの閾値を取得します。
	/// @return スリープの閾値を返します。
	PfxFloat getSleepThreshold() const {return m_sleepVelocity;}

	///
	/// @~English
	/// @brief Get time step
	/// @details Get time step.
	/// @return Returns time step.
	/// @~Japanese
	/// @brief タイムステップの取得
	/// @details タイムステップを取得します。
	/// @return タイムステップを返します。
	PfxFloat getTimeStep() const {return m_timeStep;}

	///
	/// @~English
	/// @brief Get weight factor to fix positional error of contact constraints
	/// @details Get weight factor for contact constraints.
	/// @return Returns weight factor for contact constraints.
	/// @~Japanese
	/// @brief 衝突位置を補正するためのウェイト値の取得
	/// @details 衝突のウェイト値を取得します。
	/// @return 衝突のウェイト値を返します。
	PfxFloat getSeparateWeight() const { return m_separateWeight; }

	///
	/// @~English
	/// @brief Get iteration count
	/// @details Get iteration count.
	/// @return Returns iteration count.
	/// @~Japanese
	/// @brief 反復回数の取得
	/// @details 反復回数を取得します。
	/// @return 反復回数を返します。
	PfxUInt32 getVelocityIterationCount(PfxUInt32 i) const { return m_velocityIteration[i%3]; }
	PfxUInt32 getPositioinIterationCount() const { return m_positionIteration; }

	///
	/// @~English
	/// @brief Get simulation flag
	/// @details Get simulation flag.
	/// @return Returns simulation flag.
	/// @~Japanese
	/// @brief シミュレーションフラグの取得
	/// @details シミュレーションフラグを取得します。
	/// @return シミュレーションフラグを返します。
	PfxUInt32 getSimulationFlag() const {return m_simulationFlag;}
	
	///
	/// @~English
	/// @brief Get segment width
	/// @details Get segment width.
	/// @return Returns segment width.
	/// @~Japanese
	/// @brief セグメント幅の取得
	/// @details セグメント幅を取得します。
	/// @return セグメント幅を返します。
	PfxInt32 getSegmentWidth() const {return m_segmentWidth;}
	
	///
	/// @~English
	/// @brief Get the number of worker threads
	/// @details Get the number of worker threads.
	/// @return Returns the number of worker threads.
	/// @~Japanese
	/// @brief ワーカースレッドの数の取得
	/// @details ワーカースレッドの数を取得します。
	/// @return ワーカースレッドの数を返します。
	PfxUInt32 getNumWorkerThreads() const {return m_numWorkerThreads;}

	///
	/// @~English
	/// @brief Get multithread flag
	/// @details Get multithread flag.
	/// @return Returns multithread flag.
	/// @~Japanese
	/// @brief マルチスレッドフラグの取得
	/// @details マルチスレッドフラグを取得します。
	/// @return マルチスレッドフラグを返します。
	PfxUInt32 getMultiThreadFlag() const {return m_multiThreadFlag;}
	
	///
	/// @~English
	/// @brief Set the world minimum position
	/// @details Set the minimum position of the world.
	/// @param worldMin The world minimum position
	/// @~Japanese
	/// @brief ワールドの最小座標値の設定
	/// @details ワールドの最小座標値を設定します。
	/// @param worldMin ワールドの最小座標値
	void setWorldMin(const PfxLargePosition &worldMin) {m_worldMin = worldMin;}

	///
	/// @~English
	/// @brief Set world maximum position
	/// @details Set the maximum position of the world.
	/// @param worldMax The world maximum position
	/// @~Japanese
	/// @brief ワールドの最大座標値の設定
	/// @details ワールドの最大座標値を設定します。
	/// @param worldMax ワールドの最大座標値
	void setWorldMax(const PfxLargePosition &worldMax) {m_worldMax = worldMax;}

	///
	/// @~English
	/// @brief Set sleep threshold
	/// @details Set sleep threshold.
	/// @param threshold Sleep threshold
	/// @~Japanese
	/// @brief スリープの閾値の設定
	/// @details スリープの閾値を設定します。
	/// @param threshold スリープの閾値
	void setSleepThreshold(PfxFloat threshold) {m_sleepVelocity = threshold;}

	///
	/// @~English
	/// @brief Set gravity
	/// @details Set gravity.
	/// @param gravity Gravity
	/// @~Japanese
	/// @brief 重力の設定
	/// @details 重力を設定します。
	/// @param gravity 重力
	void setGravity(const PfxVector3 &gravity) {m_gravity = gravity;}

	///
	/// @~English
	/// @brief Set time step
	/// @details Set time step.
	/// @param timeStep Time step
	/// @~Japanese
	/// @brief タイムステップの設定
	/// @details タイムステップを設定します。
	/// @param timeStep タイムステップ
	void setTimeStep(PfxFloat timeStep) {m_timeStep = timeStep;}

	///
	/// @~English
	/// @brief Set bias factor for contact constraints
	/// @details Set bias factor for contact constraints.
	/// @param bias Bais factor
	/// @~Japanese
	/// @brief 衝突のバイアス値の設定
	/// @details 衝突のバイアス値を設定します。
	/// @param bias バイアス値
	void setSeparateWeight(PfxFloat weight) {m_separateWeight = weight;}

	///
	/// @~English
	/// @brief Set iteration count
	/// @details Set iteration count.
	/// @param iteration Iteration count
	/// @~Japanese
	/// @brief 反復回数の設定
	/// @details 反復回数を設定します。
	/// @param iteration 反復回数
	void setVelocityIterationCount(PfxUInt32 i, PfxUInt32 iteration) {if(i<=3) m_velocityIteration[i] = iteration;}
	void setPositioinIterationCount(PfxUInt32 iteration) {m_positionIteration = iteration;}
	
	///
	/// @~English
	/// @brief Set simulation flag
	/// @details Set simulation flag.
	/// @param flag Simulation flag
	/// @~Japanese
	/// @brief シミュレーションフラグの設定
	/// @details シミュレーションフラグを設定します。
	/// @param flag シミュレーションフラグ
	void setSimualtionFlag(PfxUInt32 flag) {m_simulationFlag = flag;}

	/// @}
	
	/// @name Rigid body operation
	/// @{
	
	/// 
	/// @~English
	/// @brief Add a rigid body
	/// @details Create a rigid body and submit to the world.
	/// @param state Rigid body state
	/// @param body Rigid body
	/// @param collidable Collidable shape
	/// @return Returns rigid body ID. If there is no space, an assertion is called.
	/// @~Japanese
	/// @brief 剛体の作成
	/// @details 剛体を作成し、ワールドへ登録します。
	/// @param state 剛体ステート
	/// @param body 剛体属性
	/// @param collidable 剛体形状
	/// @return 剛体IDを返します。もし、空きがなければアサートが呼ばれます。
	inline PfxUInt32 addRigidBody(const PfxRigidState &state,const PfxRigidBody &body,const PfxCollidable &collidable);
	
	/// 
	/// @~English
	/// @brief Add a joint
	/// @details Create a joint and submit to the world.
	/// @param joint Joint
	/// @return Returns joint ID. If there is no space, an assertion is called.
	/// @~Japanese
	/// @brief ジョイントの作成
	/// @details ジョイントを作成し、ワールドへ登録します。
	/// @param joint ジョイント
	/// @return ジョイントIDを返します。もし、空きがなければアサートが呼ばれます。
	inline PfxUInt32 addJoint(const PfxJoint &joint);
	
	///
	/// @~English
	/// @brief Update a joint
	/// @details This method updates a related joint pair.
	/// Call when joint parameters are changed.
	/// @param jointId Joint ID
	/// @~Japanese
	/// @brief ジョイントの更新
	/// @details このメソッドは関連するジョイントペアを更新します。
	/// ジョイントのパラメータを変更した場合に呼び出してください。
	/// @param jointId ジョイントID
	inline void updateJoint(PfxUInt32 jointId);
	
	///
	/// @~English
	/// @brief Remove a rigid body
	/// @details Remove a rigid body from the world.
	/// It doesn't decrease the number of rigid bodies. ID is stored to create a next rigid body.
	/// @param rigidBodyId Rigid body ID
	/// @~Japanese
	/// @brief 剛体の削除
	/// @details 剛体をワールドから削除します。
	/// 剛体の数は変化しません。IDは次に剛体が作成されるときのため保持されます。
	/// @param rigidBodyId 剛体ID
	inline void removeRigidBody(PfxUInt32 rigidBodyId);
	
	///
	/// @~English
	/// @brief Remove a joint
	/// @details Remove a joint from the world.
	/// It doesn't decrease the number of joints. ID is stored to create a next joint.
	/// @param jointId Joint ID
	/// @~Japanese
	/// @brief ジョイントの削除
	/// @details ジョイントをワールドから削除します。
	/// ジョイントの数は変化しません。IDは次にジョイントが作成されるときのため保持されます。
	/// @param jointId ジョイントID
	inline void removeJoint(PfxUInt32 jointId);
	
	///
	/// @~English
	/// @brief Check rigid body ID
	/// @details Check if the specified rigid body ID is now in use.
	/// @param rigidBodyId Rigid body ID
	/// @return Returns false if the ID is in use.
	/// @~Japanese
	/// @brief 剛体IDのチェック
	/// @details 指定された剛体IDが使用中かどうかを調べます。
	/// @param rigidBodyId 剛体ID
	/// @return IDが使用中の場合はfalseを返します。
	inline bool isRemovedRigidBody(PfxUInt32 rigidBodyId) const;

	///
	/// @~English
	/// @brief Check joint ID
	/// @details Check if the specified joint ID is now in use.
	/// @param jointId Joint ID
	/// @return Returns false if the ID is in use.
	/// @~Japanese
	/// @brief ジョイントIDのチェック
	/// @details 指定されたジョイントIDが使用中かどうかを調べます。
	/// @param jointId ジョイントID
	/// @return IDが使用中の場合はfalseを返します。
	inline bool isRemovedJoint(PfxUInt32 jointId) const;

	///
	/// @~English
	/// @brief Get the number of rigid bodies
	/// @details Get the number of rigid bodies.
	/// @return Returns the number of rigid bodies.
	/// @~Japanese
	/// @brief 剛体数の取得
	/// @details 剛体数を取得します。
	/// @return 剛体数を返します。
	PfxUInt32 getRigidBodyCount() const {return m_states.length();}

	///
	/// @~English
	/// @brief Get the maximum capacity of rigid bodies
	/// @details Get the maximum capacity of rigid bodies.
	/// @return Returns the maximum capacity of rigid bodies.
	/// @~Japanese
	/// @brief 剛体の最大容量の取得
	/// @details 剛体の最大容量を取得します。
	/// @return 剛体の最大容量を返します。
	PfxUInt32 getRigidBodyCapacity() const {return m_states.capacity();}

	///
	/// @~English
	/// @brief Get the number of shapes
	/// @details Get the number of shapes.
	/// @return Returns the number of shapes.
	/// @~Japanese
	/// @brief 形状数の取得
	/// @details 形状数を取得します。
	/// @return 形状数を返します。
	PfxUInt32 getShapeCount() const {return m_shapes.length();}

	///
	/// @~English
	/// @brief Get the maximum capacity of shapes
	/// @details Get the maximum capacity of shapes.
	/// @return Returns the maximum capacity of shapes.
	/// @~Japanese
	/// @brief 形状の最大容量の取得
	/// @details 形状の最大容量を取得します。
	/// @return 形状の最大容量を返します。
	PfxUInt32 getShapeCapacity() const {return m_shapes.capacity();}

	///
	/// @~English
	/// @brief Get the number of joints
	/// @details Get the number of joints.
	/// @return Returns the number of joints
	/// @~Japanese
	/// @brief ジョイント数の取得
	/// @details ジョイント数を取得します。
	/// @return ジョイント数を返します。
	PfxUInt32 getJointCount() const {return m_joints.length();}
	
	///
	/// @~English
	/// @brief Get the maximum capacity of joints
	/// @details Get the maximum capacity of joints.
	/// @return Returns the maximum capacity of joints
	/// @~Japanese
	/// @brief ジョイントの最大容量の取得
	/// @details ジョイントの最大容量を取得します。
	/// @return ジョイントの最大容量を返します。
	PfxUInt32 getJointCapacity() const {return m_joints.capacity();}

	///
	/// @~English
	/// @brief Get the number of contacts
	/// @details Get the number of contacts.
	/// @return Returns the number of contacts
	/// @~Japanese
	/// @brief 衝突数の取得
	/// @details 衝突数を取得します。
	/// @return 衝突数を返します。
	PfxUInt32 getContactCount()	const {return m_numPairs[m_pairSwap];}

	///
	/// @~English
	/// @brief Get the maximum capacity of contacts
	/// @details Get the maximum capacity of contacts.
	/// @return Returns the maximum capacity of contacts.
	/// @~Japanese
	/// @brief 衝突の最大容量の取得
	/// @details 衝突の最大容量を取得します。
	/// @return 衝突の最大容量を返します。
	PfxUInt32 getContactCapacity() const {return m_maxPairs;}

	///
	/// @~English
	/// @brief Get a rigid body state
	/// @details Get a rigid body state.
	/// @param rigidBodyId Rigid body ID
	/// @return Returns a rigid body state.
	/// @~Japanese
	/// @brief 剛体ステートの取得
	/// @details 剛体ステートを取得します。
	/// @param rigidBodyId 剛体ID
	/// @return 剛体ステートを返します。
	PfxRigidState &getRigidState(PfxUInt32 rigidBodyId) {SCE_PFX_ALWAYS_ASSERT(rigidBodyId<m_states.length());return m_states[rigidBodyId];}

	const PfxRigidState	&getRigidState(PfxUInt32 rigidBodyId) const {SCE_PFX_ALWAYS_ASSERT(rigidBodyId<m_states.length());return m_states[rigidBodyId];}

	///
	/// @~English
	/// @brief Get a rigid body attribute
	/// @details Get a rigid body attribute.
	/// @param rigidBodyId Rigid body ID
	/// @return Returns a rigid body attribute.
	/// @~Japanese
	/// @brief 剛体属性の取得
	/// @details 剛体属性を取得します。
	/// @param rigidBodyId 剛体ID
	/// @return 剛体属性を返します。
	PfxRigidBody &getRigidBody(PfxUInt32 rigidBodyId) {SCE_PFX_ALWAYS_ASSERT(rigidBodyId<m_bodies.length());return m_bodies[rigidBodyId];}

	const PfxRigidBody &getRigidBody(PfxUInt32 rigidBodyId) const {SCE_PFX_ALWAYS_ASSERT(rigidBodyId<m_bodies.length());return m_bodies[rigidBodyId];}
	
	///
	/// @~English
	/// @brief Get a collidable shape
	/// @details Get a collidable shape.
	/// @param rigidBodyId Rigid body ID
	/// @return Returns a collidable shape.
	/// @~Japanese
	/// @brief 剛体形状の取得
	/// @details 剛体形状を取得します。
	/// @param rigidBodyId 剛体ID
	/// @return 剛体形状を返します。
	PfxCollidable &getCollidable(PfxUInt32 rigidBodyId) {SCE_PFX_ALWAYS_ASSERT(rigidBodyId<m_collidables.length());return m_collidables[rigidBodyId];}

	const PfxCollidable &getCollidable(PfxUInt32 rigidBodyId) const {SCE_PFX_ALWAYS_ASSERT(rigidBodyId<m_collidables.length());return m_collidables[rigidBodyId];}
	
	///
	/// @~English
	/// @brief Get a joint
	/// @details Get a joint.
	/// @param jointId Joint ID
	/// @return Returns a joint.
	/// @~Japanese
	/// @brief ジョイントの取得
	/// @details ジョイントを取得します。
	/// @param jointId ジョイントID
	/// @return ジョイントを返します。
	PfxJoint &getJoint(PfxUInt32 jointId) {SCE_PFX_ALWAYS_ASSERT(jointId<m_joints.length());return m_joints[jointId];}

	const PfxJoint &getJoint(PfxUInt32 jointId) const {SCE_PFX_ALWAYS_ASSERT(jointId<m_joints.length());return m_joints[jointId];}
	
	///
	/// @~English
	/// @brief Get a contact manifold
	/// @details Get a contact manifold.
	/// @param contactId Contact ID
	/// @param[out] rigidbodyIdA Rigid Body A ID
	/// @param[out] rigidbodyIdB Rigid Body B ID
	/// @return Returns a contact manifold.
	/// @~Japanese
	/// @brief 衝突情報の取得
	/// @details 衝突情報を取得します。
	/// @param contactId 衝突ID
	/// @param[out] rigidbodyIdA 剛体AのID
	/// @param[out] rigidbodyIdB 剛体BのID
	/// @return 衝突情報を返します。
	const PfxContactManifold &getContactManifold(PfxUInt32 contactId,PfxUInt32 &rigidbodyIdA,PfxUInt32 &rigidbodyIdB) const;
	
	/// 
	/// @~English
	/// @brief Get a rigid body array
	/// @details Get a pointer to the array of rigid body states.
	/// @return Returns a pointer to the array of rigid body states.
	/// @~Japanese
	/// @brief 剛体ステート配列の取得
	/// @details 剛体ステート配列へのポインタを取得します。
	/// @return 剛体ステート配列へのポインタを返します。
	PfxRigidState *getRigidStatePtr() {return m_states.ptr();}
	
	/// 
	/// @~English
	/// @brief Get a rigid body attribute array
	/// @details Get a pointer to the array of rigid body attributes.
	/// @return Returns a pointer to the array of rigid body attributes.
	/// @~Japanese
	/// @brief 剛体属性配列の取得
	/// @details 剛体属性配列へのポインタを取得します。
	/// @return 剛体属性配列へのポインタを返します。
	PfxRigidBody *getRigidBodyPtr() {return m_bodies.ptr();}
	
	/// 
	/// @~English
	/// @brief Get a collidable shape array
	/// @details Get a pointer to the array of collidable shapes.
	/// @return Returns a pointer to the array of collidable shapes.
	/// @~Japanese
	/// @brief 剛体形状配列の取得
	/// @details 剛体形状配列へのポインタを取得します。
	/// @return 剛体形状配列へのポインタを返します。
	PfxCollidable *getCollidablePtr() {return m_collidables.ptr();}
	
	/// 
	/// @~English
	/// @brief Get a joint array
	/// @details Get a pointer to the array of joint states.
	/// @return Returns a pointer to the array of joint states.
	/// @~Japanese
	/// @brief ジョイント配列の取得
	/// @details ジョイント配列へのポインタを取得します。
	/// @return ジョイント配列へのポインタを返します。
	PfxJoint *getJointPtr() {return m_joints.ptr();}
	
	/// 
	/// @~English
	/// @brief Get a pair array
	/// @details Get a pointer to the array of pairs.
	/// @return Returns a pointer to the array of pairs.
	/// @~Japanese
	/// @brief ペア配列の取得
	/// @details ペア配列へのポインタを取得します。
	/// @return ペア配列へのポインタを返します。
	PfxBroadphasePair *getContactPairPtr() {return m_pairsBuff[m_pairSwap];}
	
	/// 
	/// @~English
	/// @brief Setup collidable shape
	/// @details Initialize the collidable shape with the specified number of shapes.
	/// @param collidable Collidable shape
	/// @param numShapes The number of shapes
	/// @~Japanese
	/// @brief 剛体形状のセットアップ
	/// @details 剛体形状を指定された形状数で初期化します。
	/// @param collidable 剛体形状
	/// @param numShapes 形状数
	inline void setupCollidable(PfxCollidable &collidable,PfxUInt32 numShapes);

	/// 
	/// @~English
	/// @brief Release collidable shape
	/// @details Release the collidable shape.
	/// @param collidable Collidable shape
	/// @~Japanese
	/// @brief 剛体形状の解放
	/// @details 剛体形状を解放します。
	/// @param collidable 剛体形状
	inline void releaseCollidable(PfxCollidable &collidable);

	/// @}
};

#include "pfx_rigidbody_world_implementation.h"

} // namespace PhysicsEffects
} // namespace sce

#endif // _SCE_PFX_RIGIDBODY_WORLD_H
