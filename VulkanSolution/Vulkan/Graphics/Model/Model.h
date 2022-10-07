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
#include <assimp/Importer.hpp>

struct aiNode;
struct aiScene;

class Model
{
public:
	Model(const std::string& path);
	bool LoadModel(const std::string& path);

	void* GetVertexData();
	int GetVertexCount();

	void* GetIndexData();
	int GetIndexCount();
	
	bool IsModelValid();

	const char* GetErrorString();

	// Return matrix to transfrom model in [-1,-1,-1] and [1,1,1]
	glm::mat4 CalculateAdjustBoundingBoxMatrix();
private:
	void ClearData();

	void ReadMesh(aiNode* node, const aiScene* scene);

	void UpdateBoundingBox(glm::vec3 vertex);
	glm::vec3 GetModelScale();
	glm::vec3 GetModelCentroid();

	std::vector<Vertex> vertices;

	std::vector<unsigned int> indices;

	bool isModelValid;

	Assimp::Importer importer;

	glm::vec3 boundingBox[2];
};