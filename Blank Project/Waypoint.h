#pragma once

#include "../nclgl/Vector3.h"

class Waypoint {
public:
	Waypoint() {};
	Waypoint(Vector3 worldPos) : worldPosition(worldPos) {};
	Waypoint(Vector3 worldPos, float camYaw, float camPitch, float rotTime) : worldPosition(worldPos), yaw(camYaw), pitch(camPitch), rotationTime(rotTime) {};
	~Waypoint() {};

	Vector3 GetWorldPosition() const { return worldPosition; }
	float GetYaw() { return yaw; }
	float GetPitch() { return pitch; }
	float GetRotationTime() { return rotationTime; }
protected:
	Vector3 worldPosition;
	float yaw;
	float pitch;
	float rotationTime;
};