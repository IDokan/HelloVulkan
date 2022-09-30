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

class MyVulkan
{
public:
	MyVulkan(Window* window);

	// Return if initialization is succeed or not
	// Use VK_MAKE_VERSION(major, minor, patch) for second parameter 'appVersion'
	bool InitVulkan(const char* appName, uint32_t appVersion);
	void CleanVulkan();

	void DrawFrame();

	void CreateImages();
	void CreateCubeImages();

	double linearTosRGB(double cl);
	double sRGBToLinear(double cs);

	void FillBufferWithFloats(VkCommandBuffer cmdBuffer, VkBuffer dstBuffer, VkDeviceSize offset, VkDeviceSize length, const float value);

	/*
	void CreateSwapChain();

	void TransitionImageLayout();

	void SavingPipelineCacheDataToFile(VkDevice device, VkPipelineCache cache, const char* fileName);

	void CreateDescriptorSetLayout();

	void CreatePipelineLayout();
	*/

	// void CreateSimpleRenderpass();

	//void CreateSimpleGraphicsPipeline();

	void DescribeVertexInputData();

	void SetupSeparateVertexAttribute();

private:
	bool CreateInstance(const char* appName, uint32_t appVersion);
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

	void CreateGraphicsPipeline();
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

	void RecordClientData();
	void UpdateCurrentFrameID();

	void RecreateSwapchain();
	void CleanupSwapchainResourcesForRecreation();

	void CreateBuffers();
	void CreateVertexBuffer(int vertexCount, void* vertexData);
	void DestroyBuffersAndFreeMemories();
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
	
	void CreateIndexBuffer(int indexCount, void* indexData);

	void CreateDescriptorSetLayout();
	void DestroyDescriptorSetLayout();

	void CreateUniformBuffers();
	void UpdateUniformBuffer(uint32_t currentImage);

	void CreateDescriptorPool();
	void DestroyDescriptorPool();
	void CreateDescriptorSets();

	void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

	void CreateDepthResources();
	void DestroyDepthResources();
	VkFormat FindDepthFormat();
	bool HasStencilComponent(VkFormat format);
	VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	private:
		const int MAX_FRAMES_IN_FLIGHT = 2;
private:
	Window* windowHolder;
	VkInstance instance{};
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

	VkRenderPass renderPass;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;
	std::vector<VkFramebuffer>  swapchainFramebuffers;

	// synchronization objects
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	
	uint32_t currentFrameID;

	// GLFW provides required instance extensions.

	std::vector<const char*> instanceLayers = {
		"VK_LAYER_KHRONOS_validation"		// it assists developers in isolating incorrect usage, and in verifying that applications correctly use the API.
	};

	std::vector<const char*> reqDeviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};

	uint32_t indexCount;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;

	Model* model;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;
};