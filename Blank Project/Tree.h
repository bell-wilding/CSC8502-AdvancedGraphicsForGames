#pragma once

#include "../nclgl/SceneNode.h"

class Tree : public SceneNode {
public:
	Tree(Mesh* mesh, Shader* shader);
	~Tree(void) {};

	void Draw(const OGLRenderer& r) override;
};

