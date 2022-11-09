/******************************************************************************
Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   Model.h
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 09.27.2022
	header file for model.
******************************************************************************/
#pragma once
#include <string>
#include <Graphics/Structures/Structs.h>
#include "assimp/Importer.hpp"
#include "fbxsdk.h"

struct aiNode;
struct aiScene;
class AnimationSystem;

class Model
{
public:
	Model(const std::string& path);
	~Model();
	bool LoadModel(const std::string& path);

	int GetMeshSize();

	std::string GetMeshName(int i);
	void* GetVertexData(int i);
	int GetVertexCount(int i);

	void* GetIndexData(int i);
	int GetIndexCount(int i);
	
	bool IsModelValid();

	std::string GetBoneName(unsigned int boneID);
	size_t GetBoneCount();
	void* GetBoneDataForDrawing();
	void GetToBoneFromUnit(std::vector<glm::mat4>& data);
	void GetToModelFromBone(std::vector<glm::mat4>& data);

	// @@ Getter&Setter of animation system
	unsigned int GetAnimationCount();
	unsigned int GetSelectedAnimationIndex();
	void SetAnimationIndex(unsigned int i);
	void GetAnimationData(float t, std::vector<glm::mat4>& data, bool bindPoseFlag = false);
	std::string GetAnimationName();
	float GetAnimationDuration();
	// @@ End of getter & setter.

	const char* GetErrorString();

	// Return matrix to transfrom model in [-1,-1,-1] and [1,1,1]
	glm::mat4 CalculateAdjustBoundingBoxMatrix();

	const std::vector<std::string>& GetDiffuseImagePaths();
	const std::vector<std::string>& GetNormalImagePaths();
private:
	// @@ Printing fbx data
	void ExploreScene(FbxScene* scene);
	void PrintNode(FbxNode* node);
	void PrintTabs();
	void PrintAttribute(FbxNodeAttribute* attribute);
	FbxString GetAttributeTypeName(FbxNodeAttribute::EType type);
	// @@ End of printing

	// @@ Get Mesh data
	void GetScene(FbxNode* node = nullptr);
	void GetMesh(FbxNode* node);
	bool GetMeshData(FbxMesh* mesh, Mesh& m);
	// @@ End of getting mesh

	// @@ Get Animation Data
	void GetSkeleton(FbxNode* root = nullptr);
	void InitBoneData();
	void GetAnimation();
	void AddTracksRecursively(FbxNode* node, double frameRate, double startTime, double endTime, int keyFrames);
	// @@ End of animation data


	void GetTextureData(FbxSurfaceMaterial* material);

	void ClearData();
	void CleanFBXResources();

	void ReadMesh(aiNode* node, const aiScene* scene);
	void ReadMaterial(const aiScene* scene, const std::string& path);

	void UpdateBoundingBox(glm::vec3 vertex);
	glm::vec3 GetModelScale();
	glm::vec3 GetModelCentroid();

	bool isModelValid;

	// Assimp::Importer importer;

	glm::vec3 boundingBox[2];

	std::vector<std::string> diffuseImagePaths;
	std::vector<std::string> normalImagePaths;

	FbxManager* lSdkManager;
	FbxIOSettings* ios;
	FbxImporter* lImporter;
	FbxScene* lScene;
	int numTabs = 0;

	std::vector<Mesh> meshes;

	// it would be std::vector<LineVertex> bones;
	std::vector<glm::vec3> bones;
	AnimationSystem* animationSystem;
	
	Assimp::Importer importer;
};