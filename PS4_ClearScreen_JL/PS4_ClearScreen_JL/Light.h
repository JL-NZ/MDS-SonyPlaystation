#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "Utils.h"

class Light {
	public:
		Light();
		~Light();
		static Light* GetMain();

		Vector3 m_Position = Vector3(0, 1, 0);
		Vector3 m_Colour = Vector3(1);
		float m_fLightSpecularStrength = 0.5f;
		float m_fAmbientStrength = 0.2f;

	private:
		static Light* s_pMain;


};

#endif

