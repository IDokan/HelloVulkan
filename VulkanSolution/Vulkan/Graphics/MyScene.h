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
	
	bool InitScene();
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

	void CreateBuffers();
	void ResizeModelBuffers(int size);
	void CreateVertexBuffer(VkDeviceSize bufferSize, void* vertexData, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void DestroyBuffersAndFreeMemories();
	void CreateModelBuffers();
	void DestroyModelBuffers();
	void DestroyBuffer(VkBuffer& buffer);
	void FreeMemory(VkDeviceMemory memory);

	
	
	void CreateIndexBuffer(int indexCount, void* indexData, int i);

	void CreateUniformBuffers();
	void InitUniformBufferData();
	void UpdateUniformBuffer(uint32_t currentImage);

	void CreateDescriptorSet();
	void WriteDescriptorSet();
	void DestroyDescriptorSet();

	void CreateWaxDescriptorSet();
	void WriteWaxDescriptorSet();
	void DestroyWaxDescriptorSet();


	bool HasStencilComponent(VkFormat format);

	void RecordDrawSkeletonCall(VkCommandBuffer commandBuffer);
	void RecordDrawMeshCall(VkCommandBuffer commandBuffer, VkPipeline pipeline, VkPipelineLayout pipelineLayout, DescriptorSet* descriptorSet);

	// @@@@@ Texture functions & resources
	void CreateEmergencyTexture();
	void DestroyEmergencyTexture();

	VkImage emergencyTextureImage;
	VkDeviceMemory emergencyTextureImageMemory;
	VkImageView emergencyTextureImageView;

	// @@ End of Texture functions
private:
	Window* windowHolder;
	
	DescriptorSet* descriptorSet;
	DescriptorSet* waxDescriptorSet;


	std::vector<uint32_t> indexCounts;

	std::vector<VkBuffer> vertexBuffers;
	std::vector<VkDeviceMemory> vertexBufferMemories;
	std::vector<VkBuffer> indexBuffers;
	std::vector<VkDeviceMemory> indexBufferMemories;
	
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	

	int meshSize;
	Model* model;

	float timer;
	glm::vec3 rightMouseCenter;
	glm::vec3 cameraPoint;
	glm::vec3 targetPoint;

	UniformBufferObject uniformData;


	// @@ Line drawing variables
	int boneSize;

	void CreateLinePipeline();
	VkPipelineLayout linePipelineLayout;
	VkPipeline linePipeline;

	void CreateSkeletonBuffer();
	void DestroySkeletonBuffer();
	VkBuffer skeletonLineBuffer;
	VkDeviceMemory skeletonLineBufferMemory;

	int animationCount;
	void UpdateAnimationUniformBuffer();
	VkDeviceSize animationUniformBufferSize;
	void CreateAnimationUniformBuffers();
	void DestroyAnimationUniformBuffers();
	std::vector<VkBuffer> animationUniformBuffers;
	std::vector<VkDeviceMemory> animationUniformBufferMemories;

	bool bindPoseFlag;
	bool showSkeletonFlag;
	

	// @@ No texture pipeline (WaxPipeline).
	VkPipeline waxPipeline;
	VkPipelineLayout waxPipelineLayout;
	// @@ End of no texture pipeline

	// @@ Blending Weights
	void CreateBlendingWeightDescriptorSet();
	void WriteBlendingWeightDescriptorSet();
	void DestroyBlendingWeightDescriptorSet();
	bool blendingWeightMode;
	DescriptorSet* blendingWeightDescriptorSet;
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
};