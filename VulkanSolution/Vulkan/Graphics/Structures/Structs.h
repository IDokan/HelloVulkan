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
#include <iostream>
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
		vertexColor = glm::vec3(1.f, 1.f, 1.f);
		texCoord = glm::vec2(0.f, 0.f);
		boneIDs = glm::ivec4(0);
		boneWeights = glm::vec4(0.f);
	}
	Vertex(glm::vec3 position, glm::vec3 normal, glm::vec3 vertexColor, glm::vec2 texCoord, glm::ivec4 boneIDs, glm::vec4 boneWeights)
		:position(position), normal(normal), vertexColor(vertexColor), texCoord(texCoord), boneIDs(boneIDs), boneWeights(boneWeights)
	{}

	Vertex(const Vertex& v)
		:position(v.position), normal(v.normal), vertexColor(v.vertexColor), texCoord(v.texCoord), boneIDs(v.boneIDs), boneWeights(v.boneWeights)
	{}

	Vertex(Vertex&& v)
		:position(v.position), normal(v.normal), vertexColor(v.vertexColor), texCoord(v.texCoord), boneIDs(v.boneIDs), boneWeights(v.boneWeights)
	{}
	Vertex& operator=(const Vertex& v)
	{
		position = v.position;
		normal = v.normal;
		vertexColor = v.vertexColor;
		texCoord = v.texCoord;
		boneIDs = v.boneIDs;
		boneWeights = v.boneWeights;
		return *this;
	}
	Vertex& operator=(Vertex&& v)
	{
		position = v.position;
		normal = v.normal;
		vertexColor = v.vertexColor;
		texCoord = v.texCoord;
		boneIDs = v.boneIDs;
		boneWeights = v.boneWeights;
		return *this;
	}

	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 vertexColor;
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
		static std::vector<VkVertexInputAttributeDescription> attributeDescriptions(6);

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
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;		// three 32-bit floats
		attributeDescriptions[2].offset = offsetof(Vertex, vertexColor);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;		// two 32-bit floats
		attributeDescriptions[3].offset = offsetof(Vertex, texCoord);

		attributeDescriptions[4].binding = 0;
		attributeDescriptions[4].location = 4;
		attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SINT;		// four 32-bit unsigned int
		attributeDescriptions[4].offset = offsetof(Vertex, boneIDs);

		attributeDescriptions[5].binding = 0;
		attributeDescriptions[5].location = 5;
		attributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;		// Four 32-bit floats
		attributeDescriptions[5].offset = offsetof(Vertex, boneWeights);

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

struct vec3Compare
{
	bool operator() (const glm::vec3& lhs, const glm::vec3& rhs) const;
};

struct vec2Compare
{
	bool operator() (const glm::vec2& lhs, const glm::vec2& rhs) const;
};

struct Mesh
{
	Mesh();
	Mesh(const std::string& name, const std::vector<uint32_t>& indices, const std::vector<Vertex>& vertices, const std::vector<Vertex>& uniqueVertices);
	Mesh(const Mesh& m);
	Mesh(Mesh&& m);
	Mesh& operator=(const Mesh& m);
	Mesh& operator=(Mesh&& m);

	std::string meshName;
	std::vector<uint32_t> indices;
	std::vector<Vertex> vertices;
	std::vector<Vertex> uniqueVertices;
	std::map<glm::vec3, std::vector<glm::vec3>, vec3Compare> normalByVertex;
	std::map<glm::vec3, std::vector<glm::vec2>, vec2Compare> uvByVertex;
};

struct Bone
{
public:
	Bone();
	Bone(std::string name, int parentID = -1, int id = -1, glm::mat4 toBoneFromModel = glm::mat4(1.f), glm::mat4 toModelFromBone = glm::mat4(1.f));
	Bone(const Bone& b);
	Bone(Bone&& b);
	virtual bool Update(float dt);
	Bone& operator=(const Bone& b);
	Bone& operator=(Bone&& b);
	virtual ~Bone();

	std::string name;
	int parentID;
	int id;
	glm::mat4 toBoneFromUnit;
	glm::mat4 toModelFromBone;
};


struct Physics
{
public:
	static bool forceApplyFlag;
	static float GravityScaler;
	const size_t springSize = 2;
public:
	Physics();
	Physics(const Physics& p);
	Physics(Physics&& p);
	Physics& operator=(const Physics& p);
	Physics& operator=(Physics&& p);

	~Physics();

	void Initialize();
	void UpdateByForce(float dt, glm::vec3 force);
	void UpdateByForce(float dt, std::vector<glm::vec3> _forces, glm::vec3 _torque);
public:
	glm::vec3 centerOfMass;
	glm::vec3 initCenterOfMass;
	// Linear forces
	glm::vec3 translation;
	std::vector<glm::vec3> linearMomentums;
	std::vector<glm::vec3> linearVelocities;
	glm::vec3 force;

	// Angular forces
	glm::mat3 rotation;
	glm::vec3 angularMomentum;
	glm::vec3 angularVelocity;
	glm::mat3 inertiaTensorInverse;
	glm::mat3 inertiaTensorObj;
	glm::vec3 torque;

	float totalMass;

	std::vector<glm::vec3> initVertices;
	std::vector<glm::vec3> vertices;

	float dampingScaler;
	float springScaler;

private:
	glm::mat3 Tilde(glm::vec3 v);
};

struct JiggleBone : public Bone
{
public:
	JiggleBone();
	JiggleBone(std::string name, int parentID = -1, int id = -1, glm::mat4 toBoneFromUnit = glm::mat4(), glm::mat4 toModelFromBone = glm::mat4(), const Bone* parentBonePtr = nullptr, const JiggleBone* childBonePtr = nullptr);
	JiggleBone(const JiggleBone& jb);
	JiggleBone(JiggleBone&& jb);
	virtual bool Update(float dt);
	void CalculateForces(glm::vec3& linearForceA, glm::vec3& linearForceB, glm::vec3& torqueForce, glm::vec3 gravityVector = glm::vec3(0.f, -1.f, 0.f), bool bindPoseFlag = false, std::vector<glm::mat4>* animationMatrix = nullptr);
	void UpdatePhysics(float dt, const std::vector<glm::vec3>& linearForces, glm::vec3 torqueForce);

	JiggleBone& operator=(const JiggleBone& jb);
	JiggleBone& operator=(JiggleBone&& jb);
	virtual ~JiggleBone();

	// given bindPoseFlag = 
	glm::vec3 GetInitialPointA(bool bindPoseFlag, std::vector<glm::mat4>* animationMatrix) const;
	glm::vec3 GetDynamicPointA(bool bindPoseFlag, std::vector<glm::mat4>* animationMatrix) const;
	glm::vec3 GetDynamicPointB(bool bindPoseFlag, std::vector<glm::mat4>* animationMatrix) const;

	glm::vec4 CalculateParentTransformationRecursively(const JiggleBone* jb, glm::vec4 firstGlobalPosition) const;

	void SetChildBonePtr(const JiggleBone* childBonePtr);

	void SetIsUpdateJigglePhysics(bool isUpdate);

	void AddVertices(const std::vector<glm::vec3>& vertices);

	bool isUpdateJigglePhysics;

	glm::mat4 customPhysicsTranslation;
	glm::mat4 customPhysicsRotation;

	Physics physics;
	const Bone* parentBonePtr;
	const JiggleBone* childBonePtr;
};

class Skeleton
{
public:
	Skeleton();
	~Skeleton();

	void Update(float dt, glm::mat4 modelMatrix = glm::mat4(1.f), bool bindPoseFlag = false, std::vector<glm::mat4>* animationMatrix = nullptr);

	void AddBone(std::string name, int parentID);
	void AddBone(Bone* newBone);
	int GetBoneIDByName(const std::string& name);
	// Return empty bone when bone ID is invalid
	const Bone* GetBoneByBoneID(int boneID);
	const Bone* GetBoneByName(const std::string& name);
	Bone& GetBoneReferenceByName(const std::string& name);
	std::string GetBoneNameByID(unsigned int boneID);
	size_t GetSkeletonSize();
	void Clear();

	void GetToBoneFromUnit(std::vector<glm::mat4>& data);
	void GetToModelFromBone(std::vector<glm::mat4>& data);

	void CleanBones();
private:
	// Helper functions that are not called outside of the struct.
	void CopyJiggleBoneVectors(std::vector<JiggleBone>& src, std::vector<JiggleBone>& dst);
	void CopyJiggleBoneVectors(std::vector<JiggleBone*>& src, std::vector<JiggleBone>& dst);
private:
	std::vector<Bone*> bones;
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

struct VertexPipelinePushConstants
{
	float pointSize;
	int vertexID;
	bool isMousePressed;
};

struct HairBonePushConstants
{
	float pointSize;
	int selectedBone;
};

struct SpherePushConstants
{
	glm::mat4 sphereBoundingMatrix;
	glm::mat4 translation;
	float radius;
};

std::ostream& operator<<(std::ostream& os, const glm::vec4& data);
std::ostream& operator<<(std::ostream& os, const glm::vec3& data);
std::ostream& operator<<(std::ostream& os, const glm::vec2& data);
std::ostream& operator<<(std::ostream& os, const glm::ivec2& data);
std::ostream& operator<<(std::ostream& os, const glm::mat3& data);

bool operator<(const glm::vec3& lhs, const glm::vec3& rhs);
bool operator>(const glm::vec3& lhs, const glm::vec3& rhs);
bool operator<(const glm::vec2& lhs, const glm::vec2& rhs);
bool operator>(const glm::vec2& lhs, const glm::vec2& rhs);

bool operator<(glm::vec3&& lhs, glm::vec3&& rhs);
bool operator>(glm::vec3&& lhs, glm::vec3&& rhs);
bool operator<(glm::vec2&& lhs, glm::vec2&& rhs);
bool operator>(glm::vec2&& lhs, glm::vec2&& rhs);