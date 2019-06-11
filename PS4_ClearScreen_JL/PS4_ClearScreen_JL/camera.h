/***********************
Bachelor of Software Engineering   Media Design School   Auckland   New Zealand
(c) 2019 Media Design School
File Name   :	camera.h
Description :	camera class header file
Author      :   Kerry Pellett BE(Hons)
Mail        :   kerry.pel7420@mediadesign.school.nz
********************/
#ifndef __CAMERA_H__
#define __CAMERA_H__

//#include "utility.h"
#include "Utils.h"

class Terrain;

class Camera {
	private:
		Vector3 m_Position = Vector3(-1.0f, 5.0f, 2.0f);

		Matrix4 m_projection;
		Matrix4 m_view;
		float m_fFOV;
		float m_fPitch = 0.0f;
		float m_fYaw = 0.0f;

		inline void UpdateViewMatrix();

		static Camera* s_pMain;

	public:
		Camera();
		Camera(float _fFoV, float _fRatio, float _fNear, float _fFar);
		~Camera();

		Vector3 m_CameraFront = Vector3(0.0f, 0.0f, 1.0f);
		Vector3 m_CameraUp = Vector3(0.0f, 1.0f, 0.0f);
		Vector3 m_CameraRight = Vector3(1.0f, 0.0f, 0.0f);

		static Camera* GetInstance();

		// Accessors | Mutators
		void SetPosition(Vector3 _position);
		Vector3 GetPosition()const;
		void UpdatePosition(Vector3 _displacement);
		void LocalTranslation(Vector3 _displacement);
		void Initialize(float _fFOV, float _fRatio, float _fNear, float _fFar);

		void PointAt(Vector3 _point);
		void RotateBy(Vector3 _Rotation);

		Matrix4 GetView()const;
		Matrix4 GetProjection()const;

		void KeepAboveTerrain(const Terrain* const _kpTerrain);
};

#endif // !__CAMERA_H__
