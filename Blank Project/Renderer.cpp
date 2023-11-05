#include "Renderer.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/Camera.h"
#include "../nclgl/PointLight.h"
#include "../nclgl/DirectionalLight.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/MeshMaterial.h"
#include "../nclgl/MeshAnimation.h"

#include "Ocean.h"
#include "Terrain.h"
#include "Tree.h"
#include "Wolf.h"

#include <algorithm>

const int POST_PASSES = 6;

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {

	if (!GenerateGameWorld()) {
		return;
	}

	light = new PointLight(Vector3(2000, 2000, 4000), Vector4(1, 1, 1, 1),
		5000, 1);

	camera = new Camera(-15.0f, 140.0f, Vector3(5478, 1765, 461), Vector3(320, 320, 320));

	std::vector<Waypoint> trackPoints = {
		Waypoint(Vector3(2241, 609, 2577), 214, -7.37, 10),
		Waypoint(Vector3(3388, 665, 3847), 250, -5.62, 4),
		Waypoint(Vector3(3794, 809, 3953), 247, -22, 3),
		Waypoint(Vector3(6734, 929, 4496), -80, -16, 12),
		Waypoint(Vector3(6630, 2274, 8431), 43, -28, 12),
		Waypoint(Vector3(3761, 1017, 7097), 127, -26, 8),
		Waypoint(Vector3(2338, 1377, 6846), 290,  -1, 6),
		Waypoint(Vector3(394, 449, 6465), 254, 3, 3),
		Waypoint(Vector3(233, 417, 2870), 254, 3, 5)
	};
	cameraTrack = Route(camera, trackPoints);

	InitialisePostProcessing();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	followingTrack = true;
	camera->ToggleControls(!followingTrack);
	keyWaitTimer = 0.0f;
	sceneTime = 0.0f;
	init = true;
}

Renderer::~Renderer(void) {
	delete camera;
	delete skyboxMesh;
	delete skyboxShader;
	delete light;
	delete processShader;
	delete processQuad;
	delete root;

	glDeleteTextures(2, bufferColourTex);
	glDeleteTextures(1, &bufferDepthTex);
	glDeleteFramebuffers(1, &bufferFBO);
	glDeleteFramebuffers(1, &processFBO);

	for (const auto& tex : activeTextures) {
		glDeleteTextures(1, &tex);
	}
	activeTextures.clear();
	for (const auto& shader : activeShaders) {
		delete shader;
	}
	activeShaders.clear();
}

void Renderer::UpdateScene(float dt) {
	sceneTime += dt;
	keyWaitTimer += dt;

	TogglePostProcess(camera->GetPosition().y < oceanHeight);

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_F) && keyWaitTimer > 1) {
		followingTrack = !followingTrack;
		camera->ToggleControls(!followingTrack);
		keyWaitTimer = 0.0f;
	}

	if (followingTrack) {
		cameraTrack.Update(dt);
	}
	camera->UpdateCamera(dt);

	root->Update(dt);
}

void Renderer::RenderScene() {
	DrawScene();
	DrawPostProcess();
	PresentScene();
}

void Renderer::BuildNodeLists(SceneNode* from)
{
	if (frameFrustum.InsideFrustum(*from)) {
		Vector3 dir = from->GetWorldTransform().GetPositionVector() - camera->GetPosition();
		from->SetCameraDistance(Vector3::Dot(dir, dir));

		if (from->GetColour().w < 1.0f) {
			transparentNodeList.push_back(from);
		}
		else {
			nodeList.push_back(from);
		}
	}

	for (vector<SceneNode*>::const_iterator i = from->GetChildIteratorStart();
		i != from->GetChildIteratorEnd(); ++i) {
		BuildNodeLists(*i);
	}
}

void Renderer::SortNodeLists()
{
	std::sort(transparentNodeList.rbegin(),
		transparentNodeList.rend(),
		SceneNode::CompareByCameraDistance);

	std::sort(nodeList.begin(),
		nodeList.end(),
		SceneNode::CompareByCameraDistance);
}

void Renderer::ClearNodeLists()
{
	transparentNodeList.clear();
	nodeList.clear();
}

void Renderer::DrawNodes()
{
	for (const auto& i : nodeList) {
		DrawNode(i);
	}
	for (const auto& i : transparentNodeList) {
		DrawNode(i);
	}
}

void Renderer::DrawNode(SceneNode* n)
{
	if (n->GetMesh()) {
		BindShader(n->GetShader());

		glUniform3fv(glGetUniformLocation(n->GetShader()->GetProgram(),
			"cameraPos"), 1, (float*)&camera->GetPosition());


		glUniform4fv(glGetUniformLocation(n->GetShader()->GetProgram(),
			"nodeColour"), 1, (float*)&n->GetColour());

		glUniform1i(glGetUniformLocation(
			n->GetShader()->GetProgram(), "diffuseTex"), 0);
		
		glUniform1i(glGetUniformLocation(
			n->GetShader()->GetProgram(), "bumpTex"), 1);

		if (n->GetTextureByType("Diffuse")) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, n->GetTextureByType("Diffuse"));
		}

		if (n->GetTextureByType("Bump")) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, n->GetTextureByType("Bump"));
		}

		UpdateShaderMatrices();
		SetShaderPointLight(*light);

		Matrix4 model = n->GetWorldTransform() *
			Matrix4::Scale(n->GetModelScale());

		glUniformMatrix4fv(glGetUniformLocation(n->GetShader()->GetProgram(),
			"modelMatrix"), 1, false, model.values);

		n->Draw(*this);
	}
}

void Renderer::RenderSkybox()
{
	glDepthMask(GL_FALSE);

	BindShader(skyboxShader);
	UpdateShaderMatrices();

	skyboxMesh->Draw();

	glDepthMask(GL_TRUE);
}

bool Renderer::LoadSkybox()
{
	skyboxShader = new Shader("skyboxVertex.glsl", "skyboxFragment.glsl");
	skyboxMesh = Mesh::GenerateQuad();

	skyboxCubeMap = SOIL_load_OGL_cubemap(
		TEXTUREDIR"tropical_west.jpg", TEXTUREDIR"tropical_east.jpg",
		TEXTUREDIR"tropical_up.jpg", TEXTUREDIR"tropical_down.jpg",
		TEXTUREDIR"tropical_south.jpg", TEXTUREDIR"tropical_north.jpg",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	return skyboxShader && skyboxMesh && skyboxCubeMap;
}

bool Renderer::GenerateGameWorld()
{
	if (!LoadSkybox()) {
		return false;
	}

	Terrain* terrain = CreateTerrain();
	if (!terrain) {
		return false;
	}
	PopulateTerrain(terrain);

	Vector3 terrainSize = terrain->GetHeightMapSize();
	Ocean* ocean = CreateOcean(terrainSize.y * 0.42f);
	if (!ocean) {
		return false;
	}

	root = new SceneNode();
	root->AddChild(terrain);
	root->AddChild(ocean);

	PlaceAnimatedWolves(terrain);
	activeMeshes.push_back(terrain->GetMesh());
	activeMeshes.push_back(ocean->GetMesh());
	return true;
}

Terrain* Renderer::CreateTerrain()
{
	Shader* terrainShader = LoadShader("TerrainVertex.glsl", "TerrainFrag.glsl");
	if (!terrainShader) {
		return false;
	}

	std::string terrainTexNames[8] = {
		"scree_albedo.jpg",
		"scree_normal.jpg",
		"sand_albedo.jpg",
		"sand_normal.jpg",
		"rock_albedo.jpg",
		"rock_normal.jpg",
		"grass_albedo.jpg",
		"grass_normal.jpg"
	};

	GLuint terrainTextures[8];
	GLuint tex;

	for (int i = 0; i < 8; ++i) {
		tex = LoadTexture(terrainTexNames[i]);
		if (!tex) {
			return false;
		}
		SetTextureRepeating(tex, true);
		terrainTextures[i] = tex;
	}
	BindShader(terrainShader);

	return new Terrain(terrainShader, terrainTextures);
}

void Renderer::PopulateTerrain(Terrain* t)
{
	const int maxTrees = 1000;
	const int maxRocks = 60;

	int  iWidth, iHeight, iChans;
	unsigned char* data = SOIL_load_image(TEXTUREDIR"island.png", &iWidth, &iHeight, &iChans, 1);

	Shader* shader = LoadShader("TexturedVertex.glsl", "TexturedFragment.glsl");
	PlaceItemsOnTerrain("Palm", t, maxTrees, shader, data, iWidth, iHeight, iChans, 0.985, 125, 256);
	shader = LoadShader("BumpVertex.glsl", "bumpFragment.glsl");
	PlaceItemsOnTerrain("rock", t, maxRocks, shader, data, iWidth, iHeight, iChans, 0.998, 100, 130);
	SOIL_free_image_data(data);
}

void Renderer::PlaceItemsOnTerrain(string objName, Terrain* t, int maxObjects, Shader* shader, unsigned char* terrainData, int  iWidth, int iHeight, int iChans, float chance, int heightLowerBound, int heightHigherBound)
{
	Mesh* mesh = Mesh::LoadFromMeshFile(objName+".msh");
	activeMeshes.push_back(mesh);
	MeshMaterial* mat = new MeshMaterial(objName+".mat");

	vector<GLuint> texIDs;

	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		const MeshMaterialEntry* matEntry =
			mat->GetMaterialForLayer(i);

		const string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		GLuint texID = LoadTexture(*filename);
		texIDs.emplace_back(texID);
		if (objName == "rock") {
			matEntry->GetEntry("Bump", &filename);
			texID = LoadTexture(*filename);
			texIDs.emplace_back(texID);
		}
	}
	delete mat;

	SceneNode* obj;
	int objCount = 0;

	for (int z = 0; z < iHeight; ++z) {
		for (int x = 0; x < iWidth; ++x) {
			int offset = (z * iWidth) + x;
			float randValue = ((double)rand() / (RAND_MAX));
			if (randValue > chance && terrainData[offset] < heightHigherBound && terrainData[offset] > heightLowerBound) {
				Vector3 position = Vector3(x, terrainData[offset] - 3, z);
				t->AddChild(objName == "rock" ? GenerateRock(mesh, shader, position, texIDs) : GenerateTree(mesh, shader, position, texIDs));
				++objCount;
				if (objCount == maxObjects) {
					break;
				}
			}
		}
		if (objCount == maxObjects) {
			break;
		}
	}
}

SceneNode* Renderer::GenerateRock(Mesh* rockMesh, Shader* rockShader, Vector3 position, vector<GLuint> texIDs)
{
	SceneNode* rock = new SceneNode(rockMesh, rockShader);
	float scale = ((float)rand() / (float)RAND_MAX * 13) + 2;
	rock->SetTransform(Matrix4::Translation(position
		* Vector3(16.0f, 5, 16.0f))
		* Matrix4::Rotation(rand() % 360 + 1, Vector3(0, 1, 0)));
	rock->SetModelScale(Vector3(scale, scale, scale));
	rock->SetBoundingRadius(80 * scale);
	rock->AddTexture("Diffuse", texIDs[0]);
	rock->AddTexture("Bump", texIDs[1]);
	return rock;
}

Tree* Renderer::GenerateTree(Mesh* treeMesh, Shader* treeShader, Vector3 position, vector<GLuint> texIDs)
{
	Tree* tree = new Tree(treeMesh, treeShader);
	float yScale = ((float)rand() / (float)RAND_MAX * 1.2) + 0.8;
	tree->SetTransform(Matrix4::Translation(position
		* Vector3(16.0f, 5, 16.0f))
		* Matrix4::Rotation(rand() % 360 + 1, Vector3(0, 1, 0)));
	tree->SetModelScale(Vector3(25, 25 * yScale, 25));
	tree->SetBoundingRadius(110 * yScale);
	for (int i = 0; i < texIDs.size(); ++i) {
		tree->AddTexture("Diffuse"+std::to_string(i), texIDs[i]);
	}
	return tree;
}

void Renderer::PlaceAnimatedWolves(Terrain* t) {
	Vector3 positions[5] = {
		Vector3(4421, 612, 4091),
		Vector3(4336, 598, 3845),
		Vector3(4077, 605, 4370),
		Vector3(4593, 620, 4238),
		Vector3(3597, 607, 3713)
	};

	float rotations[5] = {
		270,
		300,
		170,
		318,
		80
	};

	Shader* skinningShader = LoadShader("SkinningVertex.glsl", "LitTextureFragment.glsl");
	Mesh* wolfMesh = Mesh::LoadFromMeshFile("wolf.msh");
	activeMeshes.push_back(wolfMesh);
	MeshAnimation* wolfAnim = new MeshAnimation("wolf.anm");
	GLuint wolfDiffuse = LoadTexture("wolf_fur.jpg");
	GLuint wolfNormal = LoadTexture("wolf_fur_normal.jpg");
	Wolf* wolf;

	for (int i = 0; i < 5; ++i) {
		wolf = new Wolf(wolfMesh, wolfAnim, skinningShader);
		wolf->AddTexture("Diffuse", wolfDiffuse);
		wolf->AddTexture("Normal", wolfNormal);
		wolf->SetTransform(Matrix4::Translation(positions[i])
			* Matrix4::Rotation(rotations[i], Vector3(0, 1, 0)));
		wolf->SetModelScale(Vector3(80, 80, 80));
		wolf->SetBoundingRadius(150);
		t->AddChild(wolf);
	}
}

Ocean* Renderer::CreateOcean(const float planeHeight)
{
	Shader* oceanShader = LoadShader("OceanVertex.glsl", "OceanFragment.glsl");
	if (!oceanShader) {
		return false;
	}

	GLuint waterNormalA = LoadTexture("water_normal_a.png");
	GLuint waterNormalB = LoadTexture("water_normal_b.png");

	if (!waterNormalA || !waterNormalB) {
		return false;
	}
	SetTextureRepeating(waterNormalA, true);
	SetTextureRepeating(waterNormalB, true);

	Ocean* o = new Ocean(oceanShader, waterNormalA, waterNormalB, skyboxCubeMap);
	o->SetTransform(Matrix4::Translation(Vector3(0, planeHeight, 0)));
	return o;
}

void Renderer::InitialisePostProcessing() {
	processQuad = Mesh::GenerateQuad();

	sceneShader = LoadShader("TexturedVertex.glsl", "TexturedFragment.glsl");
	processShader = LoadShader("TexturedVertex.glsl", "ProcessFragment.glsl");
	
	oceanHeight = 526.0f;

	glGenTextures(1, &bufferDepthTex);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height,
		0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

	for (int i = 0; i < 2; ++i) {
		glGenTextures(1, &bufferColourTex[i]);
		glBindTexture(GL_TEXTURE_2D, bufferColourTex[i]);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}

	glGenFramebuffers(1, &bufferFBO); 
	glGenFramebuffers(1, &processFBO); 

	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D, bufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
		GL_TEXTURE_2D, bufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, bufferColourTex[0], 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
		GL_FRAMEBUFFER_COMPLETE || !bufferDepthTex || !bufferColourTex[0]) {
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawScene() {
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	BuildNodeLists(root);
	SortNodeLists();

	BindShader(sceneShader);
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f,
		(float)width / (float)height, 45.0f);
	viewMatrix = camera->BuildViewMatrix();
	frameFrustum.FromMatrix(projMatrix * viewMatrix);
	UpdateShaderMatrices();

	RenderSkybox();

	DrawNodes();
	ClearNodeLists();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawPostProcess() {
	glBindFramebuffer(GL_FRAMEBUFFER, processFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, bufferColourTex[1], 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	BindShader(processShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	UpdateShaderMatrices();

	glDisable(GL_DEPTH_TEST);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(
		processShader->GetProgram(), "sceneTex"), 0);

	glUniform1f(glGetUniformLocation(
		processShader->GetProgram(), "time"), sceneTime);

	glUniform1i(glGetUniformLocation(
		processShader->GetProgram(), "lastPass"), 0);

	for (int i = 0; i < POST_PASSES; ++i) {
		if (i == POST_PASSES - 1) {
			glUniform1i(glGetUniformLocation(
				processShader->GetProgram(), "lastPass"), 1);
		}

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, bufferColourTex[1], 0);
		glUniform1i(glGetUniformLocation(processShader->GetProgram(),
			"isVertical"), 0);
		glBindTexture(GL_TEXTURE_2D, bufferColourTex[0]);
		processQuad->Draw();

		glUniform1i(glGetUniformLocation(processShader->GetProgram(),
			"isVertical"), 1);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, bufferColourTex[0], 0);
		glBindTexture(GL_TEXTURE_2D, bufferColourTex[1]);
		processQuad->Draw();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);
}

void Renderer::PresentScene() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	BindShader(sceneShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	UpdateShaderMatrices();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferColourTex[0]);
	glUniform1i(glGetUniformLocation(
		sceneShader->GetProgram(), "diffuseTex"), 0);
	processQuad->Draw();
}

void Renderer::TogglePostProcess(bool on)
{
	BindShader(processShader);
	glUniform1i(glGetUniformLocation(processShader->GetProgram(),
		"perform"), on);
}

GLuint Renderer::LoadTexture(const std::string filename)
{
	std::string file = TEXTUREDIR + filename;
	GLuint tex = SOIL_load_OGL_texture(
		file.c_str(), SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	activeTextures.push_back(tex);
	return tex;
}

Shader* Renderer::LoadShader(const std::string vert, const std::string frag)
{
	Shader* shader = new Shader(vert, frag);
	activeShaders.push_back(shader);
	return shader;
}


