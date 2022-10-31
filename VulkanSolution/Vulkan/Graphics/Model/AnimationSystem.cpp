/******************************************************************************
Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   Animation.cpp
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 10.25.2022
	source file for animation.
******************************************************************************/
#include "AnimationSystem.h"
#include "../Structures/Structs.h"

AnimationSystem::AnimationSystem(unsigned int animationCount)
	: selectedAnimation(0), animationCount(animationCount)
{
	animations.resize(animationCount);
}

void AnimationSystem::ImportSkeleton(FbxNode* node, FbxNode* parentNode)
{
	int parentID = -1;
	if (parentNode == nullptr)
	{
		skeleton.AddBone(node->GetName(), parentID);
	}
	else
	{
		skeleton.AddBone(node->GetName(), skeleton.GetBoneIDByName(parentNode->GetName()));
	}
}

void AnimationSystem::Clear()
{
	skeleton.Clear();
	animations.clear();
}

void AnimationSystem::SetAnimationCount(unsigned int _animationCount)
{
	selectedAnimation = 0;
	animationCount = _animationCount;
	animations.resize(animationCount);
}

void AnimationSystem::SetAnimationIndex(unsigned int index)
{
	if (index >= animationCount)
	{
		index %= animationCount;
	}
	selectedAnimation = index;
}

void AnimationSystem::AddAnimation(std::string animationName, size_t skeletonCount, float duration)
{
	++animationCount;
	animations.push_back(Animation(animationName, duration, skeletonCount));
}

void AnimationSystem::AddTrack(FbxNode* node, int boneID, double frameRate, double startTime, double endTime, int keyFrameCount)
{
	FbxNodeAttribute* nodeAttribute = node->GetNodeAttribute();
	FbxTime fTime;

	std::vector<KeyFrame>& keyFrames = animations[selectedAnimation].tracks[boneID].keyFrames;
	keyFrames.resize(keyFrameCount);

	double time = 0.0;
	for (int i = 0; i < keyFrameCount; i++)
	{
		fTime.SetSecondDouble(time);

		FbxAMatrix tmp = node->EvaluateLocalTransform(fTime);
		glm::mat4 result = ConvertFbxMatrixToGLM(tmp);

		keyFrames[i].time = static_cast<float>(time);
		keyFrames[i].toModelFromBone = result;

		if (time <= endTime)
		{
			time += 1.0 / frameRate;
		}
	}
}

void AnimationSystem::GetAnimationData(float t, std::vector<glm::mat4>& data)
{
	const std::vector<Track> tracks = animations[selectedAnimation].tracks;
	size_t size = tracks.size();
	data.resize(size);
	
	for (size_t i = 0; i < size; i++)
	{
		if (int parentID = skeleton.GetBoneByBoneID(static_cast<int>(i)).parentID;
			parentID >= 0)
		{
			data[i] = data[parentID] * tracks[i].keyFrames[0].toModelFromBone;
		}
		else
		{
			data[i] = tracks[i].keyFrames[0].toModelFromBone;
		}
	}
}

void AnimationSystem::GetToBoneFromUnit(std::vector<glm::mat4>& data)
{
	skeleton.GetToBoneFromUnit(data);
}

void AnimationSystem::GetToModelFromBone(std::vector<glm::mat4>& data)
{
	skeleton.GetToModelFromBone(data);
}

size_t AnimationSystem::GetBoneCount()
{
	return skeleton.GetSkeletonSize();
}

int AnimationSystem::GetBoneIDByName(const std::string& name)
{
	return skeleton.GetBoneIDByName(name);
}

void AnimationSystem::GetDeformerData(FbxMesh* mesh, Mesh& m)
{
	const int deformerCount = mesh->GetDeformerCount();
	if (deformerCount <= 0)
	{
		return;
	}

	for (int i = 0; i < deformerCount; i++)
	{
		FbxSkin* skinDeformer = reinterpret_cast<FbxSkin*>(mesh->GetDeformer(i, FbxDeformer::eSkin));
		if (skinDeformer == nullptr)
		{
			continue;
		}

		GetClusterData(skinDeformer, m);
	}
}

void AnimationSystem::GetClusterData(FbxSkin* skin, Mesh& mesh)
{
	// temporary container for boneID, bone weights.
	// There were no appropriate solution to pass these data directly to the vertex data.
		// Set default data to -1 to this temporary container and pass only appropriate data to the real vertex data.
			// Why we should use temporary, set -1 to vertex data and pass them GPU might cause out of bound error.
	const int meshVertexCount = static_cast<int>(mesh.vertices.size());
	std::vector<glm::ivec4> boneVertexID(meshVertexCount, glm::ivec4(-1));
	std::vector<glm::vec4> boneVertexWeight(meshVertexCount);

	const int clusterCount = skin->GetClusterCount();
	// Joint == Cluster
	for (int jointID = 0; jointID < clusterCount; jointID++)
	{
		FbxCluster* cluster = skin->GetCluster(jointID);

		if (cluster == nullptr)
		{
			continue;
		}

		FbxNode* bone = cluster->GetLink();
		if (bone == nullptr)
		{
			continue;
		}

		FbxAMatrix toModelFromBone, toBoneFromUnit, meshTransform;

		// Return the transfromation of the mesh at binding time.
		cluster->GetTransformMatrix(meshTransform);
		cluster->GetTransformLinkMatrix(toBoneFromUnit);

		toBoneFromUnit = toBoneFromUnit * meshTransform;
		toModelFromBone = toBoneFromUnit.Inverse();

		Bone& t = skeleton.GetBoneReferenceByName(bone->GetName());
		t.toModelFromBone = ConvertFbxMatrixToGLM(toModelFromBone);
		t.toBoneFromUnit = ConvertFbxMatrixToGLM(toBoneFromUnit);

		const int vertexIdCount = cluster->GetControlPointIndicesCount();
		int* vertexIds = cluster->GetControlPointIndices();
		double* boneWeights = cluster->GetControlPointWeights();

		// save boneID, boneWeights to temporary container.
		for (int i = 0; i < vertexIdCount; i++)
		{
			int id = vertexIds[i];
			double weights = boneWeights[i];
			
			for (int index = 0; index < 4; index++)
			{
				if (boneVertexID[id][index] == -1)
				{
					boneVertexID[id][index] = t.id;
					boneVertexWeight[id][index] = static_cast<float>(weights);
					break;
				}
			}
		}
	}

	// Pass boneID, boneWeights data to the vertex data.
	for (int i = 0; i < meshVertexCount; i++)
	{
		for (int vectorIndex = 0; vectorIndex < 4; vectorIndex++)
		{
			if (boneVertexID[i][vectorIndex] >= 0 && mesh.vertices[i].boneIDs[vectorIndex] <= 0)
			{
				mesh.vertices[i].boneIDs[vectorIndex] = boneVertexID[i][vectorIndex];
				mesh.vertices[i].boneWeights[vectorIndex] = boneVertexWeight[i][vectorIndex];
			}
			else
			{
				mesh.vertices[i].boneIDs[vectorIndex] = 0;
				mesh.vertices[i].boneWeights[vectorIndex] = 0.f;
			}
		}
	}
}

glm::mat4 AnimationSystem::ConvertFbxMatrixToGLM(FbxAMatrix fbxMatrix)
{
	glm::mat4 result;
	result[0][0] = static_cast<float>(fbxMatrix.Get(0, 0));
	result[0][1] = static_cast<float>(fbxMatrix.Get(0, 1));
	result[0][2] = static_cast<float>(fbxMatrix.Get(0, 2));
	result[0][3] = static_cast<float>(fbxMatrix.Get(0, 3));
	result[1][0] = static_cast<float>(fbxMatrix.Get(1, 0));
	result[1][1] = static_cast<float>(fbxMatrix.Get(1, 1));
	result[1][2] = static_cast<float>(fbxMatrix.Get(1, 2));
	result[1][3] = static_cast<float>(fbxMatrix.Get(1, 3));
	result[2][0] = static_cast<float>(fbxMatrix.Get(2, 0));
	result[2][1] = static_cast<float>(fbxMatrix.Get(2, 1));
	result[2][2] = static_cast<float>(fbxMatrix.Get(2, 2));
	result[2][3] = static_cast<float>(fbxMatrix.Get(2, 3));
	result[3][0] = static_cast<float>(fbxMatrix.Get(3, 0));
	result[3][1] = static_cast<float>(fbxMatrix.Get(3, 1));
	result[3][2] = static_cast<float>(fbxMatrix.Get(3, 2));
	result[3][3] = static_cast<float>(fbxMatrix.Get(3, 3));

	return result;
}
