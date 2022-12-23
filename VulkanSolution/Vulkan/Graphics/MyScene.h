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

class Model;
class Window;
class DescriptorSet;
class Graphics;

class MyScene
{
public:
	MyScene(Window* window);
	
	bool InitScene(Graphics* graphics);
	void CleanScene();

	void DrawFrame(float dt);

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
	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	void RecordDrawModelCalls(VkCommandBuffer commandBuffer);

	void UpdateCurrentFrameID();

	void ResizeModelBuffers(int size);

	
	
	void CreateUniformBuffers();
	void InitUniformBufferData();
	void UpdateUniformBuffer(uint32_t currentImage);

	void WriteDescriptorSet();

	void WriteWaxDescriptorSet();


	bool HasStencilComponent(VkFormat format);

	void RecordDrawSkeletonCall(VkCommandBuffer commandBuffer);
	void RecordDrawMeshCall(VkCommandBuffer commandBuffer, VkPipeline pipeline, VkPipelineLayout pipelineLayout, DescriptorSet* descriptorSet);

private:
	Window* windowHolder;
	
	std::vector<uint32_t> indexCounts;

	std::vector<VkBuffer> vertexBuffers;
	std::vector<VkDeviceMemory> vertexBufferMemories;
	std::vector<VkBuffer> indexBuffers;
	std::vector<VkDeviceMemory> indexBufferMemories;
	
	Model* model;

	float timer;
	glm::vec3 rightMouseCenter;
	glm::vec3 cameraPoint;
	glm::vec3 targetPoint;

	UniformBufferObject uniformData;


	// @@ Line drawing variables
	int boneSize;

	VkPipelineLayout linePipelineLayout;
	VkPipeline linePipeline;

	VkBuffer skeletonLineBuffer;
	VkDeviceMemory skeletonLineBufferMemory;

	int animationCount;
	void UpdateAnimationUniformBuffer();

	bool bindPoseFlag;
	bool showSkeletonFlag;
	

	// @@ No texture pipeline (WaxPipeline).
	VkPipeline waxPipeline;
	VkPipelineLayout waxPipelineLayout;
	// @@ End of no texture pipeline

	// @@ Blending Weights
	void WriteBlendingWeightDescriptorSet();
	bool blendingWeightMode;
	VkPipeline blendingWeightPipeline;
	VkPipelineLayout blendingWeightPipelineLayout;


	int selectedBone;
	// @@ End of blending

	void RecordPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlagBits targetStage, void* data, uint32_t dataSize);
	void RecordPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlagBits targetStage, void const* data, uint32_t dataSize);


	bool showModel;
	bool vertexPointsMode;
	float pointSize;
	VkPipeline vertexPointsPipeline;
	VkPipelineLayout vertexPointsPipelineLayout;

	Graphics* graphics;
	std::vector<Object*> graphicResources;
private:
	Object* FindObjectByName(std::string name);

};