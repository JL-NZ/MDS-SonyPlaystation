/* SIE CONFIDENTIAL
 * PlayStation(R)4 Programmer Tool Runtime Library Release 05.008.001
 *                Copyright (C) 2016 Sony Interactive Entertainment Inc.
 *                                                All Rights Reserved.
 */

#ifndef	_SCE_PFX_RIGIDBODY_WORLD_IMPLEMENTATION_H
#define _SCE_PFX_RIGIDBODY_WORLD_IMPLEMENTATION_H

inline
const PfxContactManifold &PfxRigidBodyWorld::getContactManifold(PfxUInt32 pairId,PfxUInt32 &rigidbodyIdA,PfxUInt32 &rigidbodyIdB) const
{
	const PfxContactManifold *contact = pfxContactContainerGetContactManifold(m_contactContainer,m_pairsBuff[m_pairSwap][pairId]);
	SCE_PFX_ASSERT(contact);
	rigidbodyIdA = pfxGetObjectIdA(m_pairsBuff[m_pairSwap][pairId]);
	rigidbodyIdB = pfxGetObjectIdB(m_pairsBuff[m_pairSwap][pairId]);
	return *contact;
}

inline
PfxUInt32 PfxRigidBodyWorld::addRigidBody(const PfxRigidState &state,const PfxRigidBody &body,const PfxCollidable &collidable)
{
	PfxUInt32 id = m_states.push(state);
	m_bodies.push(body);
	m_collidables.push(collidable);
	m_states[id].setRigidBodyId(id);
	return id;
}

inline
PfxUInt32 PfxRigidBodyWorld::addJoint(const PfxJoint &joint)
{
	PfxUInt32 id = m_joints.push(joint);
	m_jointPairs.push(PfxConstraintPair());
	pfxUpdateJointPairs(m_jointPairs[id],id,m_joints[id],
		m_states[m_joints[id].m_rigidBodyIdA],m_states[m_joints[id].m_rigidBodyIdB]);
	return id;
}

inline
void PfxRigidBodyWorld::updateJoint(PfxUInt32 jointId)
{
	pfxUpdateJointPairs(m_jointPairs[jointId],jointId,m_joints[jointId],
		m_states[m_joints[jointId].m_rigidBodyIdA],m_states[m_joints[jointId].m_rigidBodyIdB]);
}

inline void PfxRigidBodyWorld::removeRigidBody(PfxUInt32 rigidBodyId)
{
	//J	半径１の球体の固定剛体として、ワールド境界の外に配置して次回再利用に備える。
	//E Change a removed rigid body as a fixed sphere (radius=1.0f), and place outside of the world 
	//E preparing for the next use.
	PfxLargePosition outOfWorld = m_worldMax;
	outOfWorld.offset += PfxVector3(1.5f);
	m_states[rigidBodyId].setMotionType(kPfxMotionTypeFixed);
	m_states[rigidBodyId].setLargePosition(outOfWorld);
	releaseCollidable(m_collidables[rigidBodyId]);

	m_states.remove(rigidBodyId);
	m_bodies.remove(rigidBodyId);
	m_collidables.remove(rigidBodyId);
}

inline void PfxRigidBodyWorld::removeJoint(PfxUInt32 jointId)
{
	m_joints[jointId].m_active = 0;
	pfxSetActive(m_jointPairs[jointId],false);
	m_joints.remove(jointId);
	m_jointPairs.remove(jointId);
}

inline bool PfxRigidBodyWorld::isRemovedRigidBody(PfxUInt32 rigidBodyId) const
{
	return m_states.isRemoved(rigidBodyId);
}

inline bool PfxRigidBodyWorld::isRemovedJoint(PfxUInt32 jointId) const
{
	return m_joints.isRemoved(jointId);
}

inline
void PfxRigidBodyWorld::setupCollidable(PfxCollidable &collidable,PfxUInt32 numShapes)
{
	PfxUInt16 ids[SCE_PFX_NUMPRIMS+1];
	if (numShapes > 1) {
		for(PfxUInt32 i = 0;i<numShapes - 1;i++) {
			ids[i] = m_shapes.push(PfxShape());
		}
		collidable.reset(m_shapes.ptr(),ids,numShapes);
	}
	else {
		collidable.reset();
	}
}

inline
void PfxRigidBodyWorld::releaseCollidable(PfxCollidable &collidable)
{
	for (PfxUInt32 i = 0; i<collidable.getNumExternalShape(); i++) {
		m_shapes.remove(collidable.getExternalShapeId(i));
	}

	PfxShape shape;
	shape.reset();
	shape.setSphere(PfxSphere(1.0f));
	collidable.reset();
	collidable.addShape(shape);
	collidable.finish();
}
#endif//_SCE_PFX_RIGIDBODY_WORLD_IMPLEMENTATION_H
