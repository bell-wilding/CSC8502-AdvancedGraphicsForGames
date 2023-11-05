#pragma once

#include "Light.h"
#include "Vector4.h"
#include "Vector3.h"

class DirectionalLight : public Light {
public:
	DirectionalLight() {}
	DirectionalLight(const Vector3& direction, const Vector4& colour, float intensity = 1) : Light(colour, intensity) {
		this->direction = direction;
	}

	~DirectionalLight(void) {}

	Vector3 GetDirection() const			{ return direction; }
	void SetDirection(const Vector3& val)	{ direction = val; }

protected:
	Vector3 direction;
};