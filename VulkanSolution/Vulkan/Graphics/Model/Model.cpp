/******************************************************************************
Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   Model.cpp
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 09.27.2022
	Source file for custom structures.
******************************************************************************/
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Graphics/Model/Model.h"

Model::Model(const std::string& path)
	: isModelValid(true)
{
	LoadModel(path);
}

bool Model::LoadModel(const std::string& path)
{
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenNormals);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		isModelValid = false;
		return isModelValid;
	}

	ClearData();
	if (scene->HasMeshes())
	{
		ReadMesh(scene->mRootNode, scene);
	}
	isModelValid = true;
	return isModelValid;
}

void* Model::GetVertexData()
{
	return reinterpret_cast<void*>(vertices.data());
}

int Model::GetVertexCount()
{
	return static_cast<int>(vertices.size());
}

void* Model::GetIndexData()
{
	return reinterpret_cast<void*>(indices.data());
}

int Model::GetIndexCount()
{
	return static_cast<int>(indices.size());
}

bool Model::IsModelValid()
{
	return isModelValid;
}

const char* Model::GetErrorString()
{
	return importer.GetErrorString();
}

glm::mat4 Model::CalculateAdjustBoundingBoxMatrix()
{
	glm::vec3 modelScale = GetModelScale();
	float largest = std::max(std::max(modelScale.x, modelScale.y), modelScale.z);
	return glm::scale(glm::vec3(2.f / largest)) * glm::translate(-GetModelCentroid());
}

void Model::ClearData()
{
	vertices.clear();
	indices.clear();

	boundingBox[0] = glm::vec3(INFINITY);
	boundingBox[1] = glm::vec3(-INFINITY);
}

void Model::ReadMesh(aiNode* node, const aiScene* scene)
{
	// Process all the node's meshes (if any)
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		unsigned int baseIndex = vertices.size();
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		for(unsigned int j = 0; j < mesh->mNumVertices; j++)
		{ 
			Vertex vertex;
			vertex.position = glm::vec3(mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z);
			vertex.normal = glm::vec3(mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z);
			vertices.push_back(vertex);
			UpdateBoundingBox(vertices.back().position);
		}
		for (unsigned int j = 0; j < mesh->mNumFaces; j++)
		{
			aiFace face = mesh->mFaces[j];
			for (unsigned int k = 0; k < face.mNumIndices; k++)
			{
				indices.push_back(face.mIndices[k] + baseIndex);
			}
		}
	}
	
	// then do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ReadMesh(node->mChildren[i], scene);
	}
}

void Model::UpdateBoundingBox(glm::vec3 vertex)
{
	boundingBox[0].x = std::min(boundingBox[0].x, vertex.x);
	boundingBox[0].y = std::min(boundingBox[0].y, vertex.y);
	boundingBox[0].z = std::min(boundingBox[0].z, vertex.z);
	boundingBox[1].x = std::max(boundingBox[1].x, vertex.x);
	boundingBox[1].y = std::max(boundingBox[1].y, vertex.y);
	boundingBox[1].z = std::max(boundingBox[1].z, vertex.z);
}

glm::vec3 Model::GetModelScale()
{
	return boundingBox[1] - boundingBox[0];
}

glm::vec3 Model::GetModelCentroid()
{
	return (boundingBox[0] + boundingBox[1]) * 0.5f;
}