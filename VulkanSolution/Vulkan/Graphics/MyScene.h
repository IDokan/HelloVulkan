/******************************************************************************
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   MyScene.h
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 06.12.2021
	header file for my vulkan.
******************************************************************************/
#pragma once

#include <fstream> // for ifstream to read spv file
#include "Graphics/Structures/Structs.h"
#include <Engines/Objects/Object.h>


class Model;
class Window;
class DescriptorSet;
class Graphics;
class HairBone;

class MyScene
{
public:
	MyScene(Window* window);
	
	bool InitScene(Graphics* graphics);
	void CleanScene();

	void DrawFrame(float dt, VkCommandBuffer commandBuffer, uint32_t currentFrameID);

	void CreateCubeImages();

	double linearTosRGB(double cl);
	double sRGBToLinear(double cs);

	void FillBufferWithFloats(VkCommandBuffer cmdBuffer, VkBuffer dstBuffer, VkDeviceSize offset, VkDeviceSize length, const float value);

	void LoadNewModel();

	void InitGUI();
	void UpdateTimer(float dt);

private:


	// It has drawing triangle part, which does not make sense.
	// I'm gonna change it.
	void RecordDrawModelCalls(VkCommandBuffer commandBuffer);

	void CreateUniformBuffers();
	void InitUniformBufferData();
	void UpdateUniformBuffer(uint32_t currentFrameID);

	glm::vec3 GetMousePositionInWorldSpace(float targetZ);
	glm::vec3 GetProjectionVectorFromCamera();

	void WriteDescriptorSet();

	void WriteWaxDescriptorSet();


	bool HasStencilComponent(VkFormat format);

	void RecordDrawSkeletonCall(VkCommandBuffer commandBuffer);

	int GetSelectedVertexID(int selectedMesh);

private:
	Window* windowHolder;
	
	Model* model;

	float timer;
	glm::vec3 rightMouseCenter;
	glm::vec3 cameraPoint;
	glm::vec3 targetPoint;

	UniformBufferObject uniformData;


	// @@ Line drawing variables
	void UpdateAnimationUniformBuffer(uint32_t currentFrameID);

	bool bindPoseFlag;
	bool showSkeletonFlag;
	

	// @@ Blending Weights
	void WriteBlendingWeightDescriptorSet();
	bool blendingWeightMode;


	int selectedBone;
	// @@ End of blending

	void RecordPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlagBits targetStage, void* data, uint32_t dataSize);
	void RecordPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlagBits targetStage, void const* data, uint32_t dataSize);


	bool showModel;
	bool vertexPointsMode;
	float pointSize;
	int selectedMesh;

	Graphics* graphics;
	std::vector<Object*> graphicResources;

	float mouseSensitivity;

	HairBone* hairBone0;
	void WriteHairBoneDescriptorSet();
	void RecordDrawHairBoneCall(VkCommandBuffer commandBuffer);
	void UpdateHairBoneBuffer(uint32_t currentFrameID);
private:
	Object* FindObjectByName(std::string name);

	bool applyingBone;
	void ModifyBone();

};