#include "Route.h"
#include "../nclgl/Camera.h"

Route::Route(Camera* c, std::vector<Waypoint> waypoints) {
	this->cam = c;
	this->waypoints = waypoints;
	this->activeWaypoint = this->waypoints.front();
	distTolerance = 20.0f;
	rotateTimer = 0.0f;
}

void Route::Update(float dt) {
	rotateTimer += dt;

	Vector3 direction = activeWaypoint.GetWorldPosition() - cam->GetPosition();
	cam->SetPosition(cam->GetPosition() + direction.Normalised() * dt * cam->GetSpeed().z);

	if (rotateTimer < activeWaypoint.GetRotationTime()) {
		cam->SetYaw(Lerp(cam->GetYaw(), activeWaypoint.GetYaw(), rotateTimer / activeWaypoint.GetRotationTime() * dt));
		cam->SetPitch(Lerp(cam->GetPitch(), activeWaypoint.GetPitch(), rotateTimer / activeWaypoint.GetRotationTime() * dt));
	}

	if (direction.Length() < distTolerance) {
		waypoints.erase(waypoints.begin());
		waypoints.emplace_back(activeWaypoint);
		activeWaypoint = waypoints.front();
		rotateTimer = 0.0f;
	}
}
