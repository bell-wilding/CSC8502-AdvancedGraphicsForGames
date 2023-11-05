#pragma once

#include "Light.h"
#include "Vector4.h"
#include "Vector3.h"

class PointLight : public Light {
public:
	PointLight(const Vector3& position, const Vector4& colour, float radius, float intensity = 1) : Light(colour, intensity) {
		this->position = position;
		this->radius = radius;
	}

	~PointLight(void) {}

	Vector3 GetPosition() const				{ return position; }
	void SetPosition(const Vector3& val)	{ position = val; }

	float GetRadius() const					{ return radius; }
	void SetRadius(float val)				{ radius = val; }

protected:
	Vector3 position;
	float radius;
};

