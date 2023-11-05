#pragma once

#include "Matrix4.h"
#include "Vector3.h"

class Camera {
public:
	Camera(void) {
		yaw		= 0.0f;
		pitch	= 0.0f;
		speed	= Vector3(60, 60, 60);
		controlsEnabled = true;
	}

	Camera(float pitch, float yaw, Vector3 position, Vector3 speed = Vector3(60, 60, 60)) {
		this->pitch		= pitch;
		this->yaw		= yaw;
		this->position	= position;
		this->speed		= speed;
		controlsEnabled = true;
	}

	~Camera(void) {};

	void UpdateCamera(float dt = 1.0f);

	Matrix4 BuildViewMatrix();

	Vector3 GetPosition() const		{ return position; }
	void SetPosition(Vector3 val)	{ position = val; }

	Vector3 GetSpeed() const		{ return speed; }
	void SetSpeed(Vector3 val)		{ speed = val; }

	float GetYaw() const			{ return yaw; }
	void SetYaw(float y)			{ yaw = y; }

	float GetPitch() const			{ return pitch; }
	void SetPitch(float p)			{ pitch = p; }

	void ToggleControls(bool on)	{ controlsEnabled = on; }

protected:
	float		yaw;
	float		pitch;
	Vector3		position; // Set to 0, 0, 0 by Vector3 constructor
	Vector3		speed;
	bool		controlsEnabled;
};