#pragma once

#include "../nclgl/OGLRenderer.h"
#include "../nclgl/Frustum.h"
#include "Route.h"
#include <string>

class SceneNode;
class Camera;
class Shader;
class HeightMap;
class Terrain;
class Ocean;
class Tree;

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);

	void RenderScene() override;
	void UpdateScene(float dt) override;

protected:
	void BuildNodeLists(SceneNode* root);
	void SortNodeLists();
	void DrawNodes();
	void DrawNode(SceneNode* n);
	void ClearNodeLists();

	void RenderSkybox();
	bool LoadSkybox();

	bool GenerateGameWorld();

	Ocean* CreateOcean(const float planeHeight);

	Terrain* CreateTerrain();
	void PopulateTerrain(Terrain* t);
	void PlaceItemsOnTerrain(std::string objName, Terrain* t, int maxObjects, Shader* shader, unsigned char* terrainData, int  iWidth, int iHeight, int iChans, float chance, int heightLowerBound, int heightHigherBound);
	SceneNode* GenerateRock(Mesh* rockMesh, Shader* rockShader, Vector3 position, vector<GLuint> texIDs);
	Tree* GenerateTree(Mesh* treeMesh, Shader* treeShader, Vector3 position, vector<GLuint> texIDs);

	void PlaceAnimatedWolves(Terrain* t);

	void InitialisePostProcessing();
	void DrawScene();
	void DrawPostProcess();
	void PresentScene();
	void TogglePostProcess(bool on);

	GLuint LoadTexture(const std::string filename);
	Shader* LoadShader(const std::string vert, const std::string frag);

	SceneNode*			root;
	vector<SceneNode*>	transparentNodeList;
	vector<SceneNode*>	nodeList;

	vector<Shader*> activeShaders;
	vector<GLuint>	activeTextures;
	vector<Mesh*>	activeMeshes;

	PointLight* light;
	Camera*		camera;
	Frustum		frameFrustum;

	Route		cameraTrack;

	GLuint		skyboxCubeMap;
	Mesh*		skyboxMesh;
	Shader*		skyboxShader;

	float		oceanHeight;
	Mesh*		processQuad;
	Shader*		processShader;
	Shader*		sceneShader;
	GLuint		bufferFBO;
	GLuint		processFBO;
	GLuint		bufferColourTex[2];
	GLuint		bufferDepthTex;

	float		sceneTime;
	bool		followingTrack;
	float		keyWaitTimer;
};
