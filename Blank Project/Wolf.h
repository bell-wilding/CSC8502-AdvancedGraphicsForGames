#pragma once

#include "../nclgl/SceneNode.h"
#include "../nclgl/MeshAnimation.h"

class Wolf :public SceneNode {
public:
	Wolf(Mesh* m, MeshAnimation* animation, Shader* shader);
	~Wolf() { delete anim; };

	void Update(float dt) override;
	void Draw(const OGLRenderer& r) override;
protected:
	int currentFrame;
	float frameTime;
	MeshAnimation* anim;
};

