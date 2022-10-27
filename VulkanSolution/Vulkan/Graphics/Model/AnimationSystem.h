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

	void ImportSkeleton(FbxNode* node, FbxNode* parentNode = nullptr);

	void Clear();

	void SetAnimationCount(unsigned int animationCount);
	void SetAnimationIndex(unsigned int index);
	
	void AddAnimation(std::string animationName, size_t skeletonCount, float duration);
	void AddTrack(FbxNode* node, int boneID, double frameRate, double startTime, double endTime, int keyFrameCount);

	void GetAnimationData(float t, std::vector<glm::mat4>& data);

	size_t GetBoneCount();
	int FindBoneIDByName(const std::string& name);
private:

	glm::mat4 ConvertFbxMatrixToGLM(FbxAMatrix fbxMatrix);

	unsigned int selectedAnimation;

	unsigned int animationCount;
	std::vector<Animation> animations;

	Skeleton skeleton;
};