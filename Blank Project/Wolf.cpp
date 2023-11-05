#include "Wolf.h"

Wolf::Wolf(Mesh* m, MeshAnimation* animation, Shader* shader) {
	this->mesh = m;
	this->anim = animation;
	this->shader = shader;

	frameTime = 0.0f;
	// Offset animation
	currentFrame = rand() % 100;
}

void Wolf::Update(float dt) {
	frameTime -= dt;
	while (frameTime < 0.0f) {
		currentFrame = (currentFrame + 1) % anim->GetFrameCount();
		frameTime += 1.0f / anim->GetFrameRate();
	}
	SceneNode::Update(dt);
}

void Wolf::Draw(const OGLRenderer& r) {
	
	vector<Matrix4> frameMatrices;

	const Matrix4* invBindPose = mesh->GetInverseBindPose();
	const Matrix4* frameData = anim->GetJointData(currentFrame);

	for (unsigned int i = 0; i < mesh->GetJointCount(); ++i) {
		frameMatrices.emplace_back(frameData[i] * invBindPose[i]);
	}

	int j = glGetUniformLocation(shader->GetProgram(), "joints");
	glUniformMatrix4fv(j, frameMatrices.size(), false, (float*)frameMatrices.data());

	SceneNode::Draw(r);
}
