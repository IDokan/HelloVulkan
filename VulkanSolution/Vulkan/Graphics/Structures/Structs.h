/******************************************************************************
Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   Structs.h
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 09.27.2022
	header file for custom structures.
******************************************************************************/
#pragma once
#include <vector>
#include "Vulkan/vulkan.h"
#include <GLMath.h>
#include <string>
#include <map>

struct LineVertex {
	LineVertex(glm::vec3 position)
		:position(position)
	{}
	glm::vec3 position;

	static const VkVertexInputBindingDescription& GetBindingDescription()
	{
		static VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(LineVertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static const std::vector<VkVertexInputAttributeDescription>& GetAttributeDescriptions()
	{
		static std::vector<VkVertexInputAttributeDescription> attributeDescriptions(1);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;		// three 32-bit floats
		attributeDescriptions[0].offset = offsetof(LineVertex, position);

		return attributeDescriptions;
	}
};

struct Vertex {
	Vertex()
	{
		position = glm::vec3(0.f, 0.f, 0.f);
		normal = glm::vec3(0.f, 0.f, 0.f);
		texCoord = glm::vec2(0.f, 0.f);
		boneIDs = glm::ivec4(0);
		boneWeights = glm::vec4(0.f);
	}
	Vertex(glm::vec3 position, glm::vec3 normal, glm::vec2 texCoord, glm::ivec4 boneIDs, glm::vec4 boneWeights)
		:position(position), normal(normal), texCoord(texCoord), boneIDs(boneIDs), boneWeights(boneWeights)
	{}

	Vertex(const Vertex& v)
		:position(v.position), normal(v.normal), texCoord(v.texCoord), boneIDs(v.boneIDs), boneWeights(v.boneWeights)
	{}

	Vertex(Vertex&& v)
		:position(v.position), normal(v.normal), texCoord(v.texCoord), boneIDs(v.boneIDs), boneWeights(v.boneWeights)
	{}
	Vertex& operator=(const Vertex& v)
	{
		position = v.position;
		normal = v.normal;
		texCoord = v.texCoord;
		boneIDs = v.boneIDs;
		boneWeights = v.boneWeights;
		return *this;
	}
	Vertex& operator=(Vertex&& v)
	{
		position = v.position;
		normal = v.normal;
		texCoord = v.texCoord;
		boneIDs = v.boneIDs;
		boneWeights = v.boneWeights;
		return *this;
	}

	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoord;
	glm::ivec4 boneIDs;
	glm::vec4 boneWeights;

	static const VkVertexInputBindingDescription& GetBindingDescription()
	{
		static VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static const std::vector<VkVertexInputAttributeDescription>& GetAttributeDescriptions()
	{
		static std::vector<VkVertexInputAttributeDescription> attributeDescriptions(5);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;		// three 32-bit floats
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;		// three 32-bit floats
		attributeDescriptions[1].offset = offsetof(Vertex, normal);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;		// two 32-bit floats
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SINT;		// four 32-bit unsigned int
		attributeDescriptions[3].offset = offsetof(Vertex, boneIDs);

		attributeDescriptions[4].binding = 0;
		attributeDescriptions[4].location = 4;
		attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;		// Four 32-bit floats
		attributeDescriptions[4].offset = offsetof(Vertex, boneWeights);

		return attributeDescriptions;
	}
};

struct UniformBufferObject {
	UniformBufferObject()
		: model(glm::mat4()), view(glm::mat4()), proj(glm::mat4())
	{}
	UniformBufferObject(glm::mat4 model, glm::mat4 view, glm::mat4 proj)
		: model(model), view(view), proj(proj)
	{}
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

struct Mesh
{
	Mesh();
	Mesh(const std::string& name, const std::vector<uint32_t>& indices, const std::vector<Vertex>& vertices);
	Mesh(const Mesh& m);
	Mesh(Mesh&& m);
	Mesh& operator=(const Mesh& m);
	Mesh& operator=(Mesh&& m);

	std::string meshName;
	std::vector<uint32_t> indices;
	std::vector<Vertex> vertices;
};

struct Bone
{
	Bone();
	Bone(std::string name, int parentID = -1, int id = -1, glm::mat4 toBoneFromModel = glm::mat4(), glm::mat4 toModelFromBone = glm::mat4());
	Bone(const Bone& b);
	Bone(Bone&& b);
	Bone& operator=(const Bone& b);
	Bone& operator=(Bone&& b);

	std::string name;
	int parentID;
	int id;
	glm::mat4 toBoneFromUnit;
	glm::mat4 toModelFromBone;
};

class Skeleton
{
public:
	Skeleton();
	void AddBone(std::string name, int parentID);
	int GetBoneIDByName(const std::string& name);
	// Return empty bone when bone ID is invalid
	const Bone& GetBoneByBoneID(int boneID);
	const Bone& GetBoneByName(const std::string& name);
	Bone& GetBoneReferenceByName(const std::string& name);
	std::string GetBoneNameByID(unsigned int boneID);
	size_t GetSkeletonSize();
	void Clear();

	void GetToBoneFromUnit(std::vector<glm::mat4>& data);
	void GetToModelFromBone(std::vector<glm::mat4>& data);
private:
	std::vector<Bone> bones;
	int boneSize;
};

struct KeyFrame
{
	float time;
	glm::mat4 toModelFromBone;
};

struct Track
{
	std::vector<KeyFrame> keyFrames;
};

struct Animation
{
	Animation();
	Animation(std::string animationName, float duration, size_t trackSize);
	Animation(const Animation& m);
	Animation(Animation&& m);
	Animation& operator=(const Animation& m);
	Animation& operator=(Animation&& m);


	std::string animationName;
	float duration;
	std::vector<Track> tracks;
};