#include "Tree.h"

Tree::Tree(Mesh* mesh, Shader* shader) {
	this->mesh = mesh;
	this->shader = shader;
}

void Tree::Draw(const OGLRenderer& r) {
	glUniform1f(glGetUniformLocation(
		shader->GetProgram(), "alphaThreshold"), 0.5f);

	for (int i = mesh->GetSubMeshCount() - 1; i > -1; --i) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textures["Diffuse" + std::to_string(i)]);
		mesh->DrawSubMesh(i);
	}
	SceneNode::Draw(r);
}
