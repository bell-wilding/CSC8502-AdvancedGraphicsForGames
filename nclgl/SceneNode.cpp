#include "SceneNode.h"

SceneNode::SceneNode(Mesh* mesh, Shader* shader, Vector4 colour)
{
	this->mesh		= mesh;
	this->shader	= shader;
	this->colour	= colour;
	parent = NULL;
	modelScale = Vector3(1, 1, 1);
	boundingRadius = 1.0f;
	distanceFromCamera = 0.0f;
}

SceneNode::~SceneNode(void)
{
	for (unsigned int i = 0; i < children.size(); ++i) {
		delete children[i];
	}
}

void SceneNode::AddChild(SceneNode* s)
{
	if (s != this) {
		children.push_back(s);
		s->parent = this;
	}
}

void SceneNode::RemoveChild(SceneNode* s)
{
	auto it = std::find(children.begin(), children.end(), s);
	if (it != children.end()) {
		children.erase(it);
		delete s;
	}
}

void SceneNode::Update(float dt)
{
	if (parent) {
		worldTransform = parent->worldTransform * transform;
	}
	else {
		worldTransform = transform;
	}

	for (vector<SceneNode*>::iterator i = children.begin(); i != children.end(); ++i) {
		(*i)->Update(dt);
	}
}

void SceneNode::Draw(const OGLRenderer& r)
{
	if (mesh) { mesh->Draw(); }
}
