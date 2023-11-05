#include "Ocean.h"
#include "../nclgl/HeightMap.h"
#include <Blank Project/Renderer.h>

Ocean::Ocean(Shader* shader, GLuint normalA, GLuint normalB, GLuint skyboxCubeMap) {
	this->mesh = new HeightMap(TEXTUREDIR"island.png", 0);
	this->shader = shader;
	AddTexture("NormalA", normalA);
	AddTexture("NormalB", normalB);
	this->skyboxCubeMap = skyboxCubeMap;

	SetBoundingRadius(20000);

	offsetA = 40.0f;
	offsetB = -23.0f;

	time = 0;

	colour = Vector4(0.101f, 0.666f, 1.0f, 0.75f);

	waveAmplitude = 7.0f;
	waveSpeed = 100.0f;
	wavelength = 250.0f;
	waveDir = Vector2(0.3f, 0.9f);
}

void Ocean::Update(float dt) {
	time += dt;
	SceneNode::Update(dt);
}

void Ocean::Draw(const OGLRenderer& r)
{
	glUniform1f(glGetUniformLocation(
		shader->GetProgram(), "time"), time);

	glUniform1i(glGetUniformLocation(
		shader->GetProgram(), "bumpTexA"), 0);
	glUniform1i(glGetUniformLocation(
		shader->GetProgram(), "bumpTexB"), 1);
	glUniform1i(glGetUniformLocation(
		shader->GetProgram(), "cubeTex"), 2);
	glUniform1f(glGetUniformLocation(
		shader->GetProgram(), "offsetA"), offsetA);
	glUniform1f(glGetUniformLocation(
		shader->GetProgram(), "offsetB"), offsetB);

	glUniform1f(glGetUniformLocation(
		shader->GetProgram(), "amplitude"), waveAmplitude);
	glUniform1f(glGetUniformLocation(
		shader->GetProgram(), "speed"), waveSpeed);
	glUniform1f(glGetUniformLocation(
		shader->GetProgram(), "wavelength"), wavelength);
	glUniform2fv(glGetUniformLocation(
		shader->GetProgram(), "direction"), 1, (float*)&waveDir);
	glUniform4fv(glGetUniformLocation(
		shader->GetProgram(), "oceanColour"), 1, (float*)&colour);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures["NormalA"]);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textures["NormalB"]);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxCubeMap);

	SceneNode::Draw(r);
}
