//
// Bachelor of Software Engineering
// Media Design School
// Auckland
// New Zealand
//
// (c) 2005 - 2017 Media Design School
//
// File Name	:	CCamera.cpp
// Description	:	main implementation for CCamera
// Author		:	Jasper Lyons & James Monk
// Mail			:	jjlyonsschool@gmail.com & james.mon7482@mediadesign.school.nz
//

// Library Includes //

// Local Includes //

// This Includes //
#include "CCamera.h"
#include <math.h>

// Static Variables //
CCamera* CCamera::p_Self = nullptr;

// Static Function Prototypes //

// Types //
using namespace std;

// Constructor //
CCamera::CCamera()
{
	m_vec3_CameraPos = Vector3(0.0f, 0.0f, 20.0f);
	//m_vec3_CameraPos = Vector3(0.0f, 0.0f, 0.0f);

	m_vec3_TargetPos = Vector3(0.0f, 0.0f, 0.0f);
	m_fTargetPosXAngle = 0;
	m_fTargetPosYAngle = 0;

	mat4_projection = Matrix4::perspective(3.14f / 4.0f, 1920.0f / 1080.0f, 1.0f, 1000.0f);
}

// Destructor //
CCamera::~CCamera()
{
}

// Singleton functions
CCamera * CCamera::GetInstance()
{
	if (!p_Self)
	{
		p_Self = new CCamera();
	}
	return p_Self;
}

void CCamera::Destroy()
{
	delete p_Self;
	p_Self = nullptr;
}

// Regular functions
void CCamera::Process()
{
	// Look at location updating
	{
		// Clamp values		
		//m_fTargetPosXAngle;
		m_fTargetPosYAngle = std::fmin(m_fTargetPosYAngle, 1.0);
		m_fTargetPosYAngle = std::fmax(m_fTargetPosYAngle, -1.0);

		float fXAngleRadians = 0;
		float fYAngleRadians = 0;		

		// Set target position
		Vector2 temp = Vector2(sin(m_fTargetPosXAngle), cos(m_fTargetPosXAngle));
		Vector2 temp2 = Vector2(sin(m_fTargetPosYAngle), cos(m_fTargetPosYAngle));
		
		m_vec3_TargetPos = 
			m_vec3_CameraPos + 
			Vector3(temp.getX(), temp2.getX(), temp.getY()) * 10.0f;
	}

	// View
	mat4_view = Matrix4::lookAt(
		Point3(m_vec3_CameraPos.getX(), m_vec3_CameraPos.getY(), m_vec3_CameraPos.getZ()),
		Point3(m_vec3_TargetPos.getX(), m_vec3_TargetPos.getY(), m_vec3_TargetPos.getZ()),
		Vector3(0.0f, 1.0f, 0.0f));

	
	
	
}

Matrix4 CCamera::GetView()
{
	return mat4_view;
}

Matrix4 CCamera::GetProjection()
{
	return mat4_projection;
}

