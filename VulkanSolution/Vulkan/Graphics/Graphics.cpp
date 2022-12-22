/******************************************************************************
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   Graphics.cpp
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 12.19.2022
	Source file for Vulkan Graphics resources.
******************************************************************************/
#include "Graphics.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <algorithm>
#include <array>

#include <Helper/VulkanHelper.h>
#include <Engines/Window.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Graphics::Graphics()
{
}

Graphics::~Graphics()
{
}

bool Graphics::InitVulkan(const char* appName, uint32_t appVersion, Window* _windowHolder)
{
	windowHolder = _windowHolder;

	if (CreateInstance(appName, appVersion) == false)
	{
		return false;
	}
	CreatePhysicalDevice();
	ChooseQueueFamily();
	if (CreateDevice() == false)
	{
		return false;
	}
	if (CreateSurfaceByGLFW() == false)
	{
		return false;
	}
	if (CreateCommandPoolAndAllocateCommandBuffers() == false)
	{
		return false;
	}
	if (CreateSwapchain() == false)
	{
		return false;
	}
	CreateRenderPass();

	CreateSyncObjects();

	CreateImageViews();

	CreateDepthResources();

	CreateFramebuffers();

	CreateTextureSampler();

	return true;
}

void Graphics::CleanVulkan()
{
	VulkanHelper::VkCheck(vkDeviceWaitIdle(device), "failed to make logical device idle");

	DestroyTextureSampler();

	DestroyFramebuffers();

	DestroyDepthResources();

	DestroyImageViews();

	DestroySyncObjects();

	DestroyRenderPass();

	DestroySwapchain();

	DestroyCommandPool();

	DestroySurface();

	DestroyDevice();

	DestroyInstance();
}

VkDevice Graphics::GetDevice()
{
	return device;
}

bool Graphics::CreateInstance(const char* appName, uint32_t appVersion)
{
	//A generic application info structure
	VkApplicationInfo applicationInfo{};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pNext = nullptr;
	applicationInfo.pApplicationName = appName;
	applicationInfo.applicationVersion = appVersion;
	applicationInfo.pEngineName = "Sinil Engine";
	applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.apiVersion = VK_API_VERSION_1_0;

	uint32_t glfwExtensionCount;
	const char** extensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);


	VkValidationFeatureEnableEXT enables[] = { VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT };
	VkValidationFeaturesEXT features{};
	features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
	features.enabledValidationFeatureCount = 1;
	features.pEnabledValidationFeatures = enables;


	std::vector<const char*> instanceLayers = {
		"VK_LAYER_KHRONOS_validation"		// it assists developers in isolating incorrect usage, and in verifying that applications correctly use the API.
	};
	instanceLayers = LoadCompatibleLayers(instanceLayers);

	// Create an instance
	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = &features;
	instanceCreateInfo.flags = 0;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;
	instanceCreateInfo.enabledExtensionCount = glfwExtensionCount;
	instanceCreateInfo.ppEnabledExtensionNames = extensions;
	instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(instanceLayers.size());
	instanceCreateInfo.ppEnabledLayerNames = instanceLayers.data();
	if (VulkanHelper::VkCheck(vkCreateInstance(&instanceCreateInfo, nullptr, &instance), "Could not create instance") != VK_SUCCESS)
	{
		DestroyInstance();
		return false;
	}

	return true;
}

void Graphics::DestroyInstance()
{
	vkDestroyInstance(instance, nullptr);
}

void Graphics::CreatePhysicalDevice()
{
	uint32_t physicalDevicesCount;
	VulkanHelper::VkCheck(vkEnumeratePhysicalDevices(instance, &physicalDevicesCount, nullptr), "Get number of physical devices has failed.");
	std::vector<VkPhysicalDevice> physicalDeviceCandidates(physicalDevicesCount);
	VulkanHelper::VkCheck(vkEnumeratePhysicalDevices(instance, &physicalDevicesCount, physicalDeviceCandidates.data()), "Get physical devices information has failed.");

	for (const VkPhysicalDevice& physicalDeviceCandidate : physicalDeviceCandidates)
	{
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(physicalDeviceCandidate, &deviceProperties);

		if (deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			continue;
		}

		uint32_t extensionCount = 0;
		VulkanHelper::VkCheck(vkEnumerateDeviceExtensionProperties(physicalDeviceCandidate, nullptr, &extensionCount, nullptr), "Get number of device extension properties has failed.");
		std::vector<VkExtensionProperties> extensions(extensionCount);
		VulkanHelper::VkCheck(vkEnumerateDeviceExtensionProperties(physicalDeviceCandidate, nullptr, &extensionCount, extensions.data()), "Get device extension properties has failed.");

		unsigned int compatibleExtensions = 0;
		for (const char* element : reqDeviceExtensions)
		{
			for (size_t i = 0; i < extensionCount; i++)
			{
				if (strcmp(element, extensions[i].extensionName) == 0)
				{
					compatibleExtensions++;
					break;
				}
			}

			if (compatibleExtensions == reqDeviceExtensions.size())
			{
				physicalDeviceProperties = deviceProperties;
				physicalDevice = physicalDeviceCandidate;
				return;
			}
		}
	}

}

void Graphics::ChooseQueueFamily()
{
	// VK_QUEUE_GRAPHICS_BIT - queues in this family support graphics operations such as drawing points, line,s and triangles.
	// VK_QUEUE_COMPUTE_BIT - queues in this family support compute operations such as dispatching compute shaders.
	// VK_QUEUE_TRANSFER_BIT - queues in this family support transfer operations such as copying buffer and image contents.
	// VK_QUEUE_SPARSE_BINDING_BIT - queues in this family support memory binding operations used to update sparse resources.
	VkQueueFlags requiredQueueFlags = VK_QUEUE_GRAPHICS_BIT;

	uint32_t queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilyCandidates(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyCandidates.data());

	// Search the list for the first queue family that has the required flags.
	for (uint32_t i = 0; i < queueFamilyCount; i++)
	{
		// Use bit manipulation to check queue flags contain every flags we want
		if ((queueFamilyCandidates[i].queueFlags & requiredQueueFlags) == requiredQueueFlags)
		{
			queueFamily = i;
		}
	}
}

bool Graphics::CreateDevice()
{
	physicalDeviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	physicalDeviceFeatures.pNext = nullptr;

	vkGetPhysicalDeviceFeatures2(physicalDevice, &physicalDeviceFeatures);

	// priority is between [0.f, 1.f], 1.f has higher priority while 0 has lower priority.
	float priority = 1.0f;
	VkDeviceQueueCreateInfo queueCreateInfo{};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.queueFamilyIndex = queueFamily;
	queueCreateInfo.pQueuePriorities = &priority;


	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;

	deviceCreateInfo.pNext = &physicalDeviceFeatures;

	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(reqDeviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = reqDeviceExtensions.data();

	if (VulkanHelper::VkCheck(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device), "Creating a logical device has failed.") != VK_SUCCESS)
	{
		DestroyDevice();
		return false;
	}

	GetCommandQueue();

	return true;
}

// VkSurface is Vulkan's name for the screen.
// Since GLFW creates and manges the window, it creates the VkSurface at our request.
bool Graphics::CreateSurfaceByGLFW()
{
	VkBool32 isSupported;		//Supports drawing on a screen

	VulkanHelper::VkCheck(glfwCreateWindowSurface(instance, windowHolder->glfwWindow, nullptr, &surface), "Creating surface by GLFW has failed!");
	VulkanHelper::VkCheck(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamily, surface, &isSupported), "Querying if presentation is supported has failed!");

	if (isSupported != VK_TRUE)
	{
		DestroySurface();
		return false;
	}

	return true;
}

// Create a command pool used to allocate command buffers, which in turn used to gather and send commands to the GPU.
// The flag makes it possible to reuse command buffers.
// The queue index determines which queue the command buffers can be submitted to.
// Use the command pool to also create a command buffer.
bool Graphics::CreateCommandPoolAndAllocateCommandBuffers()
{
	commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandPoolCreateInfo poolCreateInfo{};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolCreateInfo.queueFamilyIndex = queueFamily;
	if (VulkanHelper::VkCheck(vkCreateCommandPool(device, &poolCreateInfo, nullptr, &commandPool), "Creating command pool has failed!") != VK_SUCCESS)
	{
		DestroyCommandPool();
		return false;
	}

	VkCommandBufferAllocateInfo bufferAllocateInfo{};
	bufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	bufferAllocateInfo.commandPool = commandPool;
	bufferAllocateInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
	bufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	if (VulkanHelper::VkCheck(vkAllocateCommandBuffers(device, &bufferAllocateInfo, commandBuffers.data()), "Allocating command buffer has failed!") != VK_SUCCESS)
	{
		DestroyCommandPool();
		return false;
	}

	return true;
}

bool Graphics::CreateSwapchain()
{
	VkSurfaceCapabilitiesKHR capabilities;
	VulkanHelper::VkCheck(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities), "Get physical device surface capabilities has failed!");

	uint32_t formatCount;
	VulkanHelper::VkCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr), "Get number of surface formats has failed!");
	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	VulkanHelper::VkCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data()), "Get surface formats has failed!");

	VkSurfaceFormatKHR selectedFormat = ChooseSwapSurfaceFormat(formats);
	// Save swapchain image format because tutorial let me do.
	swapchainImageFormat = selectedFormat.format;

	uint32_t presentModeCount;
	VulkanHelper::VkCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr), "Get number of present mode failed!");
	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	VulkanHelper::VkCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data()), "Get present mode has failed!");

	VkPresentModeKHR selectedPresentMode = ChooseSwapPresentMode(presentModes);

	swapchainExtent = ChooseSwapExtent(capabilities);

	// one more image than the minimum to get another image to render to even when internal operation image capability is full.
	uint32_t imageCount = capabilities.minImageCount + 1;

	// 0 indicates there are no maximum image count.
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
	{
		imageCount = capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	// the surface where the swapchain should be tied to.
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = selectedFormat.format;
	createInfo.imageColorSpace = selectedFormat.colorSpace;
	createInfo.imageExtent = swapchainExtent;
	// This specifies the amount of layers each image consists of, which should be always 1.
	createInfo.imageArrayLayers = 1;
	// This specifies what kind of operations we'll use the images in the swap chain for.
	// We use COLOR_ATTACHMENT_BIT to render directly to it.
	// If you want to render images to a separate image first to perform operations like post-processing. It might be TRANSFER_DST_BIT 
	//		-> Draw somewhere else image, and send to here.
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	// Like a 90 degree CW rotation or horizontal flip
	createInfo.preTransform = capabilities.currentTransform;
	// It specifies if the alpha channel should be used for blending with other windows in the window system.
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	// Queue ownership to image.
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.queueFamilyIndexCount = 1;
	createInfo.pQueueFamilyIndices = &queueFamily;
	createInfo.presentMode = selectedPresentMode;
	createInfo.oldSwapchain = VK_NULL_HANDLE;
	// If it is true, it means that we don't care about the color of pixels that are obscured.
	createInfo.clipped = VK_TRUE;

	if (VulkanHelper::VkCheck(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain), "Creating swapchain has failed!") != VK_SUCCESS)
	{
		return false;
	}

	GetSwapchainImages();

	return true;
}

void Graphics::CreateRenderPass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapchainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	// They affects color and depth data
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = VulkanHelper::FindDepthFormat(physicalDevice);
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	// The layout specifies which layout we would like the attachment to have during a subpass that uses this reference.
	// Vulkan will automatically transition the attachment to this layout when the subpass is started.
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	// @@@@@@@@ TODO: Back to dependency part and understand what dependency is for
	// https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Rendering_and_presentation
	// @@ Todo: I forgot what they do, recall it again
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments{ colorAttachment, depthAttachment };
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	VulkanHelper::VkCheck(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass), "Creating render pass has failed!");
}

void Graphics::CreateSyncObjects()
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VulkanHelper::VkCheck(
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i])
			, "Creating a synchronization object(imageAvailableSemaphore) has failed!"
		);
		VulkanHelper::VkCheck(
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i])
			, "Creating a synchronization object(renderFinishedSemaphore) has failed!"
		);
		VulkanHelper::VkCheck(
			vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i])
			, "Creating a synchronization object(inFlightFence) has failed!"
		);
	}
}

void Graphics::DestroySyncObjects()
{
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}
}

void Graphics::CreateFramebuffers()
{
	const size_t swapchainImageViewCount = swapchainImageViews.size();
	swapchainFramebuffers.resize(swapchainImageViewCount);

	for (size_t i = 0; i < swapchainImageViewCount; i++)
	{
		std::array<VkImageView, 2> attachments =
		{
			swapchainImageViews[i],
			depthImageView
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapchainExtent.width;
		framebufferInfo.height = swapchainExtent.height;
		framebufferInfo.layers = 1;

		VulkanHelper::VkCheck(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapchainFramebuffers[i]), "Creating swapchain framebuffer has failed!");
	}
}

void Graphics::DestroyFramebuffers()
{
	for (VkFramebuffer& framebuffer : swapchainFramebuffers)
	{
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}
}

void Graphics::CreateDepthResources()
{
	VkFormat depthFormat = VulkanHelper::FindDepthFormat(physicalDevice);

	CreateImage(swapchainExtent.width, swapchainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
	depthImageView = CreateImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void Graphics::DestroyDepthResources()
{
	vkDestroyImageView(device, depthImageView, nullptr);
	vkDestroyImage(device, depthImage, nullptr);
	vkFreeMemory(device, depthImageMemory, nullptr);
}

void Graphics::CreateImageViews()
{
	swapchainImageViews.resize(swapchainImages.size());

	const size_t swapchainImageCount = swapchainImages.size();
	for (size_t i = 0; i < swapchainImageCount; i++)
	{
		swapchainImageViews[i] = CreateImageView(swapchainImages[i], swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

void Graphics::DestroyImageViews()
{
	for (VkImageView& imageView : swapchainImageViews)
	{
		vkDestroyImageView(device, imageView, nullptr);
	}
}

void Graphics::CreateTextureSampler()
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	// Anisotropy filter info
	if (physicalDeviceFeatures.features.samplerAnisotropy == VK_TRUE)
	{
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = physicalDeviceProperties.limits.maxSamplerAnisotropy;
	}
	else
	{
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = 1.f;
	}

	// [0, texWidth) vs [0, 1)
	samplerInfo.unnormalizedCoordinates = VK_FALSE;

	// below info is mainly used for percentage-closer filtering on shadow maps.
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.f;
	samplerInfo.minLod = 0.f;
	samplerInfo.maxLod = 0.f;

	VulkanHelper::VkCheck(vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler), "Creating sampler has failed!");
}

void Graphics::DestroyTextureSampler()
{
	vkDestroySampler(device, textureSampler, nullptr);
}

void Graphics::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0;

	VulkanHelper::VkCheck(vkCreateImage(device, &imageInfo, nullptr, &image), "Creating image has failed!");

	VkMemoryRequirements memRequirements;
	// Note that using vkGetImageMemoryRequirements instead of vkGetBufferMemoryRequirements
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = VulkanHelper::FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

	VulkanHelper::VkCheck(vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory), "Allocating memory has failed!");

	// Note that using vkBindImageMemoryinstead of vkBindBufferMemory
	vkBindImageMemory(device, image, imageMemory, 0);
}

VkImageView Graphics::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
	VkImageViewCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = image;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = format;
	createInfo.subresourceRange.aspectMask = aspectFlags;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	VulkanHelper::VkCheck(vkCreateImageView(device, &createInfo, nullptr, &imageView), "Creating image view has failed!");

	return imageView;
}

VkCommandBuffer Graphics::BeginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void Graphics::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;


	// At this point, wait the submitted queue return back without using fence which allow me to do some other things at CPU
							// i.g. schedule multiple transfers simultaneously and wait for all of them complete, instead of executing one at a time.
	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void Graphics::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer copyCommandBuffer = BeginSingleTimeCommands();

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(copyCommandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	EndSingleTimeCommands(copyCommandBuffer);
}

void Graphics::CreateTextureImageAndImageView(const std::string& path, VkImage& textureImage, VkDeviceMemory& textureImageMemory, VkImageView& textureImageView)
{
	int texWidth, texHeight, texChannels;

	stbi_set_flip_vertically_on_load(true);
	stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	constexpr int RGBA = 4;
	VkDeviceSize imageSize = texWidth * texHeight * RGBA;

	if (!pixels)
	{
		windowHolder->DisplayMessage("Texture Error", std::string("Loading texture image has Failed!") + std::string("\n") + path);

		// The path is for when the given path is not valid, display default image.
		pixels = stbi_load("../Vulkan/Graphics/Model/EssentialImages/transparent.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		imageSize = texWidth * texHeight * RGBA;
	}

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(device, stagingBufferMemory);

	stbi_image_free(pixels);

	CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

	TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	CopyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
	// Finished creating texture image (create staging buffer, copy image data, copy buffer data to image buffer, clean staging buffer)

	// Create Image View
	textureImageView = CreateImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

void Graphics::DestroyTextureImageAndImageView(VkImage& textureImage, VkDeviceMemory textureImageMemory, VkImageView& textureImageView)
{
	vkDestroyImageView(device, textureImageView, nullptr);
	vkDestroyImage(device, textureImage, nullptr);
	vkFreeMemory(device, textureImageMemory, nullptr);
}

void Graphics::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VulkanHelper::VkCheck(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer), "Creating Vertex buffer has failed!");

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = VulkanHelper::FindMemoryTypeIndex(physicalDevice, memRequirements, properties);

	VulkanHelper::VkCheck(vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory), "Allocating vertex buffer memory has failed!");
	vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void Graphics::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = 0; // TODO
	barrier.dstAccessMask = 0; // TODO

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		std::cout << "Unsupported layout transition!" << std::endl;
		abort();
	}


	vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	EndSingleTimeCommands(commandBuffer);
}

void Graphics::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { width, height, 1 };

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	EndSingleTimeCommands(commandBuffer);
}

std::vector<const char*> Graphics::LoadCompatibleLayers(std::vector<const char*> layers)
{
	std::vector<const char*> compatibleLayers;

	uint32_t availableLayerCount;
	vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(availableLayerCount);
	vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers.data());

	for (const char* layerName : layers)
	{
		for (const VkLayerProperties& layerProperty : availableLayers)
		{
			if (strcmp(layerName, layerProperty.layerName) == 0)
			{
				compatibleLayers.push_back(layerName);
			}
		}
	}

	return compatibleLayers;
}

void Graphics::DestroyDevice()
{
	vkDestroyDevice(device, nullptr);
}

void Graphics::GetCommandQueue()
{
	// Why queueIndex is 0???
	vkGetDeviceQueue(device, queueFamily, 0, &queue);
}

void Graphics::DestroySurface()
{
	vkDestroySurfaceKHR(instance, surface, nullptr);
}

void Graphics::DestroyCommandPool()
{
	vkDestroyCommandPool(device, commandPool, nullptr);
}

void Graphics::DestroyRenderPass()
{
	vkDestroyRenderPass(device, renderPass, nullptr);
}

const VkSurfaceFormatKHR& Graphics::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) const
{
	for (const VkSurfaceFormatKHR& format : formats)
	{
		if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return format;
		}
	}

	// Return the first format if there are no appropriate format, but it's fine.
	return formats[0];
}

const VkPresentModeKHR Graphics::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& presentModes) const
{
	// For v-sync and energy saving issue, return fifo mode immediately.
	return VK_PRESENT_MODE_FIFO_KHR;

	for (const VkPresentModeKHR presentMode : presentModes)
	{
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return presentMode;
		}
	}

	// If mail box is not available, use FIFO.
	return VK_PRESENT_MODE_FIFO_KHR;
}

const VkExtent2D Graphics::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const
{
	// Vulkan tells us to match the resolution of the window.
	// However, some window managers allow us to do something different.
	//		In that case, we'll pick the resolution that best matches the window within the minImageExtent & maxImageExtent bounds.
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		int width;
		int height;
		glfwGetFramebufferSize(windowHolder->glfwWindow, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

void Graphics::RecreateSwapchain()
{
	int width = 0;
	int height = 0;
	glfwGetFramebufferSize(windowHolder->glfwWindow, &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(windowHolder->glfwWindow, &width, &height);
		glfwWaitEvents();
	}

	DestroyFramebuffers();
	DestroyImageViews();
	DestroyDepthResources();
	DestroySwapchain();

	vkDeviceWaitIdle(device);

	CreateSwapchain();
	CreateImageViews();
	CreateDepthResources();
	CreateFramebuffers();

	uniformData.proj = glm::perspective(glm::radians(45.f), swapchainExtent.width / static_cast<float>(swapchainExtent.height), 0.1f, 10.f);
	// flip the sign of the element because GLM originally designed for OpenGL, where Y coordinate of the clip coorinates is inverted.
	uniformData.proj[1][1] *= -1;
}

void Graphics::DestroySwapchain()
{
	vkDestroySwapchainKHR(device, swapchain, nullptr);
}

void Graphics::GetSwapchainImages()
{
	uint32_t imageCount;
	VulkanHelper::VkCheck(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr), "Getting number of swapchain images has failed!");
	swapchainImages.resize(imageCount);
	VulkanHelper::VkCheck(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data()), "Get swapchain images has failed!");
}
