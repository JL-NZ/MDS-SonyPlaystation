/***********************
Bachelor of Software Engineering   Media Design School   Auckland   New Zealand
(c) 2019 Media Design School
File Name   :	camera.h
Description :	Camera class implementation file
Author      :   Kerry Pellett BE(Hons)
Mail        :   kerry.pel7420@mediadesign.school.nz
********************/
#include "camera.h"
#include "Terrain.h"

Camera* Camera::s_pMain = nullptr;

Camera::Camera() {

}

Camera::Camera(float _fFOV, float _fRatio, float _fNear, float _fFar) :m_fFOV(_fFOV) {
	if (!s_pMain) {
		s_pMain = this; // Make the first camera created the main camera
	}
	m_projection = Matrix4::perspective(TORADIANS * _fFOV, _fRatio, _fNear, _fFar);
	m_view = Matrix4::lookAt(Point3(m_Position), Point3(0.0f, 0.0f, 0.0f), m_CameraUp);
}

Camera::~Camera() {

}

Matrix4 Camera::GetProjection()const {
	return m_projection;
}

Matrix4 Camera::GetView()const {
	return m_view;
}

void Camera::SetPosition(Vector3 _vecPosition) {
	m_Position = _vecPosition;
	UpdateViewMatrix();
}

Vector3 Camera::GetPosition()const {
	return m_Position;
}

void Camera::UpdatePosition(Vector3 _vecDisplacement) {
	m_Position += _vecDisplacement;
	UpdateViewMatrix();
}

void Camera::PointAt(Vector3 _vecTarget) {
	// New forward vector
	m_CameraFront = sce::Vectormath::Scalar::Aos::normalize(_vecTarget - m_Position);
	// Create right vector
	m_CameraRight = sce::Vectormath::Scalar::Aos::normalize(sce::Vectormath::Scalar::Aos::cross(Vector3(0.0f, 1.0f, 0.0f), m_CameraFront));
	// Create up vector
	m_CameraUp = sce::Vectormath::Scalar::Aos::normalize(sce::Vectormath::Scalar::Aos::cross(m_CameraFront, m_CameraRight));

	// Update view and projection
	UpdateViewMatrix();
}

void Camera::LocalTranslation(Vector3 _displacement) {
	Vector3 translation = Vector3(0); 
	translation += m_CameraRight * _displacement.getX();
	translation += m_CameraUp * _displacement.getY();
	translation += m_CameraFront * _displacement.getZ();
	UpdatePosition(translation);
}

void Camera::RotateBy(Vector3 _Rotation) {
	// Define a vector in relation to the angle
	m_fPitch -= _Rotation.getX();
	m_fYaw -= _Rotation.getY();
	std::min(m_fYaw, -89.9f);
	std::max(m_fYaw, 89.9f);
	float fPitch = m_fPitch * TORADIANS;
	float fYaw = m_fYaw * TORADIANS;
	Vector3 newForward = Vector3(-cos(fPitch) * sin(fYaw),
									sin(fPitch),
									-cos(fPitch) * cos(fYaw));

	newForward = sce::Vectormath::Scalar::Aos::normalize(newForward);

	//PointAt(newForward + m_Position);
	m_CameraFront = newForward;
		// Create right vector
	m_CameraRight = sce::Vectormath::Scalar::Aos::normalize(sce::Vectormath::Scalar::Aos::cross(Vector3(0.0f, 1.0f, 0.0f), m_CameraFront));
	// Create up vector
	m_CameraUp = sce::Vectormath::Scalar::Aos::normalize(sce::Vectormath::Scalar::Aos::cross(m_CameraFront, m_CameraRight));

	UpdateViewMatrix();
}

inline void Camera::UpdateViewMatrix() {
	m_view = Matrix4::lookAt(Point3(m_Position), Point3(m_Position + m_CameraFront), m_CameraUp);
}

Camera* Camera::GetMain() {
	return s_pMain;
}

// Ensures that the camera does not dip below the level of the terrain where it is currently at
void Camera::KeepAboveTerrain(const Terrain* const _kpTerrain) {
	// Get height of terrain at camera position
	float fTerrainHeight = _kpTerrain->GetHeight(m_Position.getX(), m_Position.getZ());
	// Make the camera the max of its current height and the terrain
	m_Position.setY(std::fmaxf(m_Position.getY(), fTerrainHeight));
}