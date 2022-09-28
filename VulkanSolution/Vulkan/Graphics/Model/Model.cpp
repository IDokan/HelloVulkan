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
{
	LoadModel(path);
}

void Model::LoadModel(const std::string& path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
		return;
	}

	ProcessNode(scene->mRootNode, scene);
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

void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
	// Process all the node's meshes (if any)
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	// for (unsigned int i = 0; i < 1; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		vertices.resize(mesh->mNumVertices);
		for(unsigned int j = 0; j < mesh->mNumVertices; j++)
		{ 
			vertices[j].position = glm::vec3(mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z);
		}
		for (unsigned int j = 0; j < mesh->mNumFaces; j++)
		{
			aiFace face = mesh->mFaces[j];
			for (unsigned int k = 0; k < face.mNumIndices; k++)
			{
				indices.push_back(face.mIndices[k]);
			}
		}
	}
	// then do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene);
	}
}
