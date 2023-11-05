#pragma once

#include "Waypoint.h"
#include <vector>

class Camera;

class Route {
public:
	Route() {};
	Route(Camera* c, std::vector<Waypoint> waypoints);
	~Route() {};

	void Update(float dt);
protected:
	float Lerp(float a, float b, float t) { return a + t * (b - a); }

	Camera* cam;
	std::vector<Waypoint> waypoints;
	Waypoint activeWaypoint;
	float distTolerance;

	float rotateTimer;
};

