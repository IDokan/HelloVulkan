/******************************************************************************
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   MyVulkan.h
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 06.12.2021
	header file for my vulkan.
******************************************************************************/
#pragma once

#include <fstream> // for ifstream to read spv file
#include "Vulkan/vulkan.h"
#include "Graphics/Allocator/Allocator.h"
#include "Graphics/Structures/Structs.h"

class Model;
class Window;
class DescriptorSet;

class MyVulkan
{
public:
	MyVulkan(Window* window);

	// Return if initialization is succeed or not
	// Use VK_MAKE_VERSION(major, minor, patch) for second parameter 'appVersion'
	bool InitVulkan(const char* appName, uint32_t appVersion);
	void CleanVulkan();

	void DrawFrame(float dt);

	void CreateImages();
	void CreateCubeImages();

	double linearTosRGB(double cl);
	double sRGBToLinear(double cs);

	void FillBufferWithFloats(VkCommandBuffer cmdBuffer, VkBuffer dstBuffer, VkDeviceSize offset, VkDeviceSize length, const float value);

	void LoadNewModel();

	void InitGUI();
	void UpdateTimer(float dt);

private:
	bool CreateInstance(const char* appName, uint32_t appVersion);
	std::vector<const char*> LoadCompatibleLayers(std::vector<const char*> layers);
	void DestroyInstance();
	void CreatePhysicalDevice();
	void ChooseQueueFamily();
	bool CreateDevice();
	void DestroyDevice();
	void GetCommandQueue();
	bool CreateSurfaceByGLFW();
	void DestroySurface();
	bool CreateCommandPoolAndAllocateCommandBuffers();
	void DestroyCommandPool();
	bool CreateSwapchain();
	const VkSurfaceFormatKHR& ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) const;
	const VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& presentModes) const;
	const VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
	void DestroySwapchain();
	void GetSwapchainImages();
	void CreateImageViews();
	void DestroyImageViews();

	void CreateRenderPass();

	void CreateGraphicsPipeline(VkShaderModule vertModule, VkShaderModule fragModule, VkDescriptorSetLayout* descriptorSetLayoutPtr, VkPipeline& pipeline, VkPipelineLayout& pipelineLayout);
	void CreateGraphicsPipeline(VkShaderModule vertModule, VkShaderModule fragModule, uint32_t pushConstantSize, VkShaderStageFlags pushConstantTargetStage, VkDescriptorSetLayout* descriptorSetLayoutPtr, VkPipeline& pipeline, VkPipelineLayout& pipelineLayout);
	static std::vector<char> readFile(const std::string& filename);
	VkShaderModule CreateShaderModule(const std::vector<char>& code);
	void DestroyPipeline();

	void CreateFramebuffers();
	void DestroyFramebuffers();

	void CreateSyncObjects();
	void DestroySyncObjects();

	// It has drawing triangle part, which does not make sense.
	// I'm gonna change it.
	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	void RecordDrawModelCalls(VkCommandBuffer commandBuffer);

	void RecordClientData();
	void UpdateCurrentFrameID();

	void RecreateSwapchain();
	void CleanupSwapchainResourcesForRecreation();

	void CreateBuffers();
	void ResizeModelBuffers(int size);
	void CreateVertexBuffer(VkDeviceSize bufferSize, void* vertexData, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void DestroyBuffersAndFreeMemories();
	void CreateModelBuffers();
	void DestroyModelBuffers();
	void DestroyBuffer(VkBuffer& buffer);
	void FreeMemory(VkDeviceMemory memory);

	// When requiredFlags are not,
	uint32_t FindMemoryTypeIndex(
		const VkMemoryRequirements& memoryRequirements);
	// returns ~0u if matched memory does not exist.
	uint32_t FindMemoryTypeIndex(
		const VkMemoryRequirements& memoryRequirements,
		VkMemoryPropertyFlags requiredFlags);

	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	
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

	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	void CreateDepthResources();
	void DestroyDepthResources();
	VkFormat FindDepthFormat();
	bool HasStencilComponent(VkFormat format);
	VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	VkCommandBuffer BeginSingleTimeCommands();
	void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

	void RecordDrawSkeletonCall(VkCommandBuffer commandBuffer);
	void RecordDrawMeshCall(VkCommandBuffer commandBuffer, VkPipeline pipeline, VkPipelineLayout pipelineLayout, DescriptorSet* descriptorSet);

	// @@@@@ Texture functions & resources
	void CreateTextures(const std::vector<std::string>& imagePaths);
	void CreateTextureImageAndImageView(const std::string& path);
	void CreateEmergencyTexture();
	void DestroyEmergencyTexture();
	void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	void CreateTextureSampler();

	void DestroyTextureImage();
	void DestroyTextureSampler();

	std::vector<VkImage> textureImages;
	std::vector<VkDeviceMemory> textureImageMemories;
	std::vector<VkImageView> textureImageViews;

	VkImage emergencyTextureImage;
	VkDeviceMemory emergencyTextureImageMemory;
	VkImageView emergencyTextureImageView;

	VkSampler textureSampler;
	// @@ End of Texture functions
private:
		const int MAX_FRAMES_IN_FLIGHT = 2;
private:
	Window* windowHolder;
	VkInstance instance{};
	VkPhysicalDeviceProperties physicalDeviceProperties;
	VkPhysicalDeviceFeatures2 physicalDeviceFeatures;
	VkPhysicalDevice physicalDevice;
	uint32_t queueFamily;
	VkDevice device; 
	VkQueue queue;
	VkSurfaceKHR surface;
	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;
	VkSwapchainKHR swapchain;
	std::vector<VkImage> swapchainImages;
	VkFormat swapchainImageFormat;
	VkExtent2D swapchainExtent;
	std::vector<VkImageView> swapchainImageViews;

	DescriptorSet* descriptorSet;
	DescriptorSet* waxDescriptorSet;
	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;
	std::vector<VkFramebuffer>  swapchainFramebuffers;

	// synchronization objects
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	
	uint32_t currentFrameID;

	// GLFW provides required instance extensions.
	VkValidationFeaturesEXT EnableBestPracticesValidation();

	std::vector<const char*> reqDeviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};

	std::vector<uint32_t> indexCounts;

	std::vector<VkBuffer> vertexBuffers;
	std::vector<VkDeviceMemory> vertexBufferMemories;
	std::vector<VkBuffer> indexBuffers;
	std::vector<VkDeviceMemory> indexBufferMemories;
	
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	

	int meshSize;
	Model* model;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

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


	PushConstants selectedBone;
	// @@ End of blending

	void RecordPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlagBits targetStage, PushConstants* data);
};