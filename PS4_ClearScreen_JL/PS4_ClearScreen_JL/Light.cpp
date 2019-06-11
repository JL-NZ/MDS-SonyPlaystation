#include "Light.h"

Light* Light::s_pMain = nullptr;

Light::Light() {
	if (!s_pMain) {
		s_pMain = this;
	}
}

Light::~Light(){}

Light* Light::GetMain() {
	return s_pMain;
}