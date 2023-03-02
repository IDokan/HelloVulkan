/******************************************************************************
Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   Animation.h
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 10.25.2022
	header file for animation.
******************************************************************************/
#pragma once
#include <vector>
#include <string>
#include "fbxsdk.h"
#include <GLMath.h>
#include "Graphics/Structures/Structs.h"

struct Animation;

class AnimationSystem
{
public:
	AnimationSystem(unsigned int animationCount = 0);
	~AnimationSystem();

	void ImportSkeleton(FbxNode* node, FbxNode* parentNode = nullptr);

	// Currently, it is only for updating jiggle bones right now..
	void Update(float dt, glm::mat4 modelMatrix = glm::mat4(1.f));

	void Clear();

	void CleanBones();

	void SetAnimationCount(unsigned int animationCount);
	unsigned int GetSelectedAnimationIndex();
	void SetAnimationIndex(unsigned int index);
	
	void AddAnimation(std::string animationName, size_t skeletonCount, float duration);
	void AddTrack(FbxNode* node, int boneID, double frameRate, double startTime, double endTime, int keyFrameCount);

	void GetAnimationData(float t, std::vector<glm::mat4>& data);

	void GetToBoneFromUnit(std::vector<glm::mat4>& data);
	void GetToModelFromBone(std::vector<glm::mat4>& data);
	size_t GetBoneCount();
	const Bone* GetBone(int boneID);
	int GetBoneIDByName(const std::string& name);
	std::string GetBoneName(unsigned int boneID);
	void AddBone(Bone* newBone);

	// Return the first bone whose parent bone is given bone ID.
	int GetChildrenBoneID(int boneID);

	// Gate function for cluster data (Gate function indicates precede function to get another data)
	void GetDeformerData(FbxMesh* mesh);

	glm::ivec4 GetBoneIndex(int vertexIndex);
	glm::vec4 GetBoneWeight(int vertexIndex);
	
	unsigned int GetAnimationCount();
	std::string GetAnimationName();
	float GetAnimationDuration();
private:

	// @@ Get Cluster (toBindPoseMatrix, bone weights, bone ID)
	void GetClusterData(FbxSkin* skin);
	// @@ End of cluster data
	glm::mat4 ConvertFbxMatrixToGLM(FbxAMatrix fbxMatrix);

	unsigned int selectedAnimation;

	unsigned int animationCount;
	std::vector<Animation> animations;

	Skeleton skeleton;

	std::vector<glm::ivec4> boneVertexID;
	std::vector<glm::vec4> boneVertexWeights;
};