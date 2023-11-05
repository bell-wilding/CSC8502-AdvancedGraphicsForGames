#pragma once

#include "Vector4.h"
#include "Vector3.h"

class Light {
public:
	Light() {}
	Light(const Vector4& colour, float intensity = 1) {
		this->colour = colour;
		this->colour.w = intensity;
	}

	~Light(void) {}

	Vector4 GetColour() const			{ return colour; }
	void SetColour(const Vector4& val)	{ colour = val; }

protected:
	Vector4 colour;
};