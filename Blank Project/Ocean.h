#pragma once

#include "../nclgl/SceneNode.h"

class Ocean : public SceneNode {
public:
	Ocean(Shader* shader, GLuint normalA, GLuint normalB, GLuint skyboxCubeMap);
	~Ocean(void) {};

	void Update(float dt) override;
	void Draw(const OGLRenderer& r) override;

protected:
	float time;

	GLuint skyboxCubeMap;
	float offsetA;
	float offsetB;

	float waveAmplitude;
	float waveSpeed;
	float wavelength;
	Vector2 waveDir;
};

