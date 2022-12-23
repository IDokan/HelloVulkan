/******************************************************************************
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   Graphics.h
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 12.19.2022
	header file for Vulkan Graphics resources.
******************************************************************************/

#include "Vulkan/vulkan.h"
#include <vector>
#include <string>

class Window;
class Buffer;
class UniformBuffer;


class Graphics
{
public:
	friend Buffer;
	friend UniformBuffer;
public:
	static constexpr unsigned int MAX_FRAMES_IN_FLIGHT = 2;
public:
	Graphics();
	~Graphics();

	// Return if initialization is succeed or not
	// Use VK_MAKE_VERSION(major, minor, patch) for second parameter 'appVersion'
	bool InitVulkan(const char* appName, uint32_t appVersion, Window* windowHolder);
	void CleanVulkan();


	VkDevice GetDevice();
	VkSampler GetTextureSampler();
	VkRenderPass GetRenderPass();
	VkExtent2D GetSwapchainExtent();

	void CreateTextureImageAndImageView(const std::string& path, VkImage& textureImage, VkDeviceMemory& textureImageMemory, VkImageView& textureImageView);
	void DestroyTextureImageAndImageView(VkImage& textureImage, VkDeviceMemory textureImageMemory, VkImageView& textureImageView);

private:
	bool CreateInstance(const char* appName, uint32_t appVersion);
	void DestroyInstance();

	void CreatePhysicalDevice();
	void ChooseQueueFamily();
	bool CreateDevice();
	bool CreateSurfaceByGLFW();
	bool CreateCommandPoolAndAllocateCommandBuffers();
	bool CreateSwapchain();
	void CreateRenderPass();

	void CreateSyncObjects();
	void DestroySyncObjects();

	void CreateFramebuffers();
	void DestroyFramebuffers();

	void CreateDepthResources();
	void DestroyDepthResources();

	void CreateImageViews();
	void DestroyImageViews();

	void CreateTextureSampler();
	void DestroyTextureSampler();

	void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);


	VkCommandBuffer BeginSingleTimeCommands();
	void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	// Texture related functions
	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	// End of texture functions
private:
	std::vector<const char*> LoadCompatibleLayers(std::vector<const char*> layers);
	void DestroyDevice();
	void GetCommandQueue();
	void DestroySurface();
	void DestroyCommandPool();
	void DestroyRenderPass();

	const VkSurfaceFormatKHR& ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) const;
	const VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& presentModes) const;
	const VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;

	void RecreateSwapchain();
	void DestroySwapchain();
	void GetSwapchainImages();


private:
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

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	VkRenderPass renderPass;
	std::vector<VkFramebuffer>  swapchainFramebuffers;

	// synchronization objects
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	uint32_t currentFrameID;

	std::vector<const char*> reqDeviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};

	 VkSampler textureSampler;

	Window* windowHolder;
};