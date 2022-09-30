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

struct aiNode;
struct aiScene;

class Model
{
public:
	Model(const std::string& path);
	void LoadModel(const std::string& path);

	void* GetVertexData();
	int GetVertexCount();

	void* GetIndexData();
	int GetIndexCount();
	
private:
	void ProcessNode(aiNode* node, const aiScene* scene);

	std::vector<Vertex> vertices;

	std::vector<unsigned int> indices;
};