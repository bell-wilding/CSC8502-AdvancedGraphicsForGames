#pragma once

#include "../nclgl/SceneNode.h"

class Terrain : public SceneNode {
public:
	Terrain(Shader* shader, GLuint* textures);
	~Terrain(void) {};

	void Draw(const OGLRenderer& r) override;
	Vector3 GetHeightMapSize() const { return mapSize; }

protected:
	GLuint textures[8];
	float minHeight;
	float maxHeight;
	float rockThreshold;
	Vector3 mapSize;
};

