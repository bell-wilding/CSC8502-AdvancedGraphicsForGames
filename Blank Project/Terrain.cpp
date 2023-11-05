#include "Terrain.h"

#include "../nclgl/HeightMap.h"

Terrain::Terrain(Shader* shader, GLuint* textures) {
	HeightMap* map = new HeightMap(TEXTUREDIR"island.png", 5);
	this->mesh = map;
	this->shader = shader;
	for (int i = 0; i < 8; ++i) {
		this->textures[i] = textures[i];
	}
	SetBoundingRadius(20000);

	mapSize = map->GetHeightMapSize();
	minHeight = -10;
	maxHeight = 800;
	rockThreshold = 0.05;
}

void Terrain::Draw(const OGLRenderer& r) {

	glEnable(GL_CULL_FACE);

	glUniform1i(glGetUniformLocation(
		shader->GetProgram(), "mudDiffuse"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glUniform1i(glGetUniformLocation(
		shader->GetProgram(), "mudBump"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textures[1]);

	glUniform1i(glGetUniformLocation(
		shader->GetProgram(), "sandDiffuse"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, textures[2]);
	glUniform1i(glGetUniformLocation(
		shader->GetProgram(), "sandBump"), 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, textures[3]);

	glUniform1i(glGetUniformLocation(
		shader->GetProgram(), "rockDiffuse"), 4);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, textures[4]);
	glUniform1i(glGetUniformLocation(
		shader->GetProgram(), "rockBump"), 5);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, textures[5]);

	glUniform1i(glGetUniformLocation(
		shader->GetProgram(), "grassDiffuse"), 6);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, textures[6]);
	glUniform1i(glGetUniformLocation(
		shader->GetProgram(), "grassBump"), 7);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, textures[7]);

	glUniform1f(glGetUniformLocation(
		shader->GetProgram(), "minHeight"), minHeight);
	glUniform1f(glGetUniformLocation(
		shader->GetProgram(), "maxHeight"), maxHeight);
	glUniform1f(glGetUniformLocation(
		shader->GetProgram(), "rockThreshold"), rockThreshold);

	SceneNode::Draw(r);
	glDisable(GL_CULL_FACE);
}
