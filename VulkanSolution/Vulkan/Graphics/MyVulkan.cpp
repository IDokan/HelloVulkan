/******************************************************************************
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   MyVulkan.cpp
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 06.12.2021
	Source file for my vulkan.
******************************************************************************/
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>	// for glfw functions such as glfwGetRequiredInstanceExtensions, glfwCreateWindowSurface, ....
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm> // for std::clamp

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Graphics/MyVulkan.h"
#include "Helper/VulkanHelper.h"
#include "GLMath.h"
#include <chrono>
#include "Engines/Window.h"
#include "Graphics/Structures/Structs.h"
#include "Graphics/Model/Model.h"
#include "ImGUI/myGUI.h"

MyVulkan::MyVulkan(Window* window)
	: windowHolder(window), currentFrameID(0), model(nullptr)
{

}

bool MyVulkan::InitVulkan(const char* appName, uint32_t appVersion)
{
	model = new Model("../Vulkan/Graphics/Model/models/stanford-bunny.fbx");

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
	// CreateTextures(model->GetDiffuseImagePaths());
	// CreateTextureSampler();
	CreateImageViews();
	CreateRenderPass();
	CreateBuffers();
	CreateDescriptorPool();
	CreateDescriptorSetLayout();
	CreateDescriptorSets();
	CreateDepthResources();
	CreateGraphicsPipeline();

	CreateFramebuffers();

	CreateSyncObjects();


	CreateImages();

	return true;
}

void MyVulkan::CleanVulkan()
{
	VulkanHelper::VkCheck(vkDeviceWaitIdle(device), "failed to make logical device idle");

	DestroySyncObjects();

	DestroyFramebuffers();

	DestroyPipeline();

	DestroyImageViews();

	DestroyDepthResources();

	DestroySwapchain();

	// DestroyTextureSampler();
	// DestroyTextureImage();

	DestroyDescriptorPool();
	DestroyDescriptorSetLayout();

	DestroyBuffersAndFreeMemories();

	DestroyCommandPool();

	// @@ TODO: Should DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr)?

	DestroySurface();

	DestroyDevice();

	DestroyInstance();

	delete model;
}

void MyVulkan::DrawFrame()
{
	// Synchronize with GPU
	vkWaitForFences(device, 1, &inFlightFences[currentFrameID], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult resultGetNextImage = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrameID], VK_NULL_HANDLE, &imageIndex);
	if (windowHolder->windowFramebufferResized || resultGetNextImage == VK_ERROR_OUT_OF_DATE_KHR)
	{
		windowHolder->SetWindowFramebufferResized(false);
		RecreateSwapchain();
		return;
	}
	else if (resultGetNextImage != VK_SUCCESS && resultGetNextImage != VK_SUBOPTIMAL_KHR)
	{
		std::cout << "Acquiring next image has failed!" << std::endl;
		abort();
	}

	UpdateUniformBuffer(currentFrameID);

	// Prevent deadlock, delay ResetFences
	vkResetFences(device, 1, &inFlightFences[currentFrameID]);

	vkResetCommandBuffer(commandBuffers[currentFrameID], 0);
	RecordCommandBuffer(commandBuffers[currentFrameID], imageIndex);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrameID] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[currentFrameID];

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrameID] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	VulkanHelper::VkCheck(vkQueueSubmit(queue, 1, &submitInfo, inFlightFences[currentFrameID]), "Submitting queue has failed!");

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapchains[] = { swapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = &imageIndex;
	// It allows you to specify an array of VkResult values to check for every individual swap chain if presentation was successful.
	// It's not necessary if you're only using a single swap chain, because you can simply use the return value of the present function.
	presentInfo.pResults = nullptr;

	VkResult resultQueuePresent = vkQueuePresentKHR(queue, &presentInfo);
	if (windowHolder->windowFramebufferResized || resultQueuePresent == VK_ERROR_OUT_OF_DATE_KHR || resultQueuePresent == VK_SUBOPTIMAL_KHR)
	{
		windowHolder->SetWindowFramebufferResized(false);
		RecreateSwapchain();
	}
	else if (resultQueuePresent != VK_SUCCESS)
	{
		std::cout << "Acquiring next image has failed!" << std::endl;
		abort();
	}

	UpdateCurrentFrameID();
}

void MyVulkan::CreateBuffers()
{
	CreateModelBuffers();
	CreateUniformBuffers();
}

void MyVulkan::CreateImages()
{
	static const VkImageCreateInfo imageCreateInfo =
	{
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,		// VkStructureType						sType 
		nullptr,																				// void*											pNext
		0,																						// VkImageCreateFlags				flags
		VK_IMAGE_TYPE_2D,													// VkImageType							imageType
		// This format specifies a four-component, 
		// 32-bit unsigned normalized format
		VK_FORMAT_R8G8B8A8_UNORM,								// VkFormat									format
		{1024, 1024, 1},																// VkExtent3D								extent
		10,																					// uint32_t										mipLevels
		1,																						// uint32_t										arrayLayers
		// single sample
		VK_SAMPLE_COUNT_1_BIT	,										// VkSampleCountFlagBits			samples
		VK_IMAGE_TILING_OPTIMAL,										// VkImageTiling							tiling
		// it is to be used as a texture
		VK_IMAGE_USAGE_SAMPLED_BIT,							// VkImageUsageFlags				usage
		VK_SHARING_MODE_EXCLUSIVE,								// VkSharingMode						sharingMode
		0,																						// uint32_t										queueFamilyIndexCount
		nullptr,																				// const uint32_t*							pQueueFamilyIndices
		VK_IMAGE_LAYOUT_UNDEFINED								// VkImageLayout						initialLayout
	};

	VkImage image = VK_NULL_HANDLE;

	VulkanHelper::VkCheck(vkCreateImage(device, &imageCreateInfo, nullptr, &image), "Creating Image is failed");

	vkDestroyImage(device, image, nullptr);
}

void MyVulkan::CreateCubeImages()
{
	VkImageCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	createInfo.imageType = VK_IMAGE_TYPE_2D;
	createInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	createInfo.extent = VkExtent3D{ 1024, 1024, 1 };
	createInfo.mipLevels = 1;
	createInfo.arrayLayers = 6;
	createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	createInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.queueFamilyIndexCount = 0;
	createInfo.pQueueFamilyIndices = nullptr;
	createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkImage cube = VK_NULL_HANDLE;

	VulkanHelper::VkCheck(vkCreateImage(device, &createInfo, nullptr, &cube), "Image creation is failed!");

	VkImageViewCreateInfo viewCreateInfo;
	viewCreateInfo;

	//vkCreateImageView(logicalDevices.front(), )

	vkDestroyImage(device, cube, nullptr);
}


double MyVulkan::linearTosRGB(double cl)
{
	double cs = 0.0;
	if (cl >= 1.0)
	{
		cs = 1.0;
	}
	else if (cl <= 0.0)
	{
		cs = 0.0;
	}
	else if (cl < 0.0031308)
	{
		cs = 12.92 * cl;
	}
	else
	{
		cs = 1.055 * pow(cl, 0.41666) - 0.55;
	}

	return cs;
}
double MyVulkan::sRGBToLinear(double cs)
{
	double cl = 0.0;
	if (cs >= 1.0)
	{
		cl = 1.0;
	}
	else if (cs <= 0.0)
	{
		cl = 0.0;
	}
	else if (cs <= 0.04045)
	{
		cl = cs / 12.92;
	}
	else
	{
		cl = pow(((cs + 0.0555) / 1.055), 2.4);
	}

	return cl;
}

void MyVulkan::FillBufferWithFloats(VkCommandBuffer cmdBuffer, VkBuffer dstBuffer, VkDeviceSize offset, VkDeviceSize size, const float value)
{
	vkCmdFillBuffer(cmdBuffer, dstBuffer, offset, size, *(const uint32_t*)&value);
}
void MyVulkan::LoadNewModel()
{
	windowHolder->isPathDropped = false;
	const char* newPath = windowHolder->path;
	
	if (model->LoadModel(newPath) == false)
	{
		windowHolder->DisplayMessage("Failed model loading!", model->GetErrorString());
		return;
	}

	// In order to clean previous model buffers successfully, 
			// I should guarantee that deleted buffers are not in use by a command buffer.
	// Thus, wait until the submitted command buffer completed execution.
	VulkanHelper::VkCheck(vkDeviceWaitIdle(device), "failed to make logical device idle");

	DestroyModelBuffers();
	CreateModelBuffers();
}
/*
void MyVulkan::CreateSwapChain()
{
	// First, we create the swap chain.
	VkSwapchainCreateInfoKHR swapchainCreateInfo =
	{
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,		// sType
		nullptr,																									// pNext
		0,																											// flags
		m_mainSurface,																					// surface
		2, 																										// minImageCount
		VK_FORMAT_R8G8B8A8_UNORM,													// imageFormat
		VK_COLORSPACE_SRGB_NONLINEAR_KHR,								// imageColorSpace
		{1024, 768},																							// imageExtent
		1,																											// imageArrayLayers
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,							// imageUsage
		VK_SHARING_MODE_EXCLUSIVE,													// imageSharingMode
		0,																											// queueFamilyIndexCount
		nullptr,																									// pQueueFamilyIndices
		VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR,							// preTransform
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,								// compositeAlpha
		VK_PRESENT_MODE_FIFO_KHR,													// presentMode
		VK_TRUE,																							// clipped
		m_swapChain																						// oldSwapchain
	};

	VulkanHelper::VkCheck(
		vkCreateSwapchainKHR(
		logicalDevices.front(),
			&swapchainCreateInfo,
			&myAllocator,
			&m_swapChain
		),
		"vkCreateSwapchainKHR is failed!");

	// Next, we query the swap chain for the number of images it actaully contains.
	uint32_t swapchainImageCount = 0;
	VulkanHelper::VkCheck(
		vkGetSwapchainImagesKHR(
			logicalDevices.front(),
			m_swapChain,
			&swapchainImageCount,
			nullptr
		),
		"vkGetSwapchainImagesKHR is failed!"
	);
	// Now we resize our image array and retrieve the image handles from the swap chain.
	m_swapChainImages.resize(swapchainImageCount);
	VulkanHelper::VkCheck(
		vkGetSwapchainImagesKHR(
			logicalDevices.front(),
			m_swapChain,
			&swapchainImageCount,
			m_swapChainImages.data()
		),
		"vkGetSwapchainImagesKHR is failed!"
	);
}

void MyVulkan::TransitionImageLayout()
{
	const VkImageMemoryBarrier barrier =
	{
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,			//	sType
		nullptr, 																							//	pNext
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,				//	srcAccessMask
		VK_ACCESS_MEMORY_READ_BIT,										//	dstAccessMask
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,		//	oldLayout
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,							//	newLayout
		0,																									//	srcQueueFamilyIndex
		0,																									//	dstQueueFamilyIndex
		sourceImage,																				//	image
		{																									//	subresourceRange
			VK_IMAGE_ASPECT_COLOR_BIT,										//	aspectMask
			0,																								//	baseMipLevel
			1,																								//	levelCount
			0,																								//	baseArrayLayer
			1																								//	layerCount
		}
	};

	vkCmdPipelineBarrier(
		cmdBuffer,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
		);
}

void MyVulkan::SavingPipelineCacheDataToFile(VkDevice device, VkPipelineCache cache, const char* fileName)
{
	size_t cacheDataSize;

	// Determine the size of the cache data.
	VulkanHelper::VkCheck(
		vkGetPipelineCacheData(device, cache, &cacheDataSize, nullptr),
		"first call to vkGetPipelineCacheData failed!"
	);

	if (cacheDataSize == 0)
	{
		// failed!
		// Handle to fail needed
		return;
	}

	FILE* pOutputFile;
	void* pData;

	// Allocate a temporary store for the cache data.
	pData = malloc(cacheDataSize);

	if (pData == nullptr)
	{
		return;
	}

	// Retrieve the actual data from the cache.
	VulkanHelper::VkCheck(
		vkGetPipelineCacheData(device, cache, &cacheDataSize, pData),
		"Second call to vkGetPipelineCacheData is failed!"
	);

	// Open the file as a binary file and write the data to it.
	pOutputFile = fopen(fileName, "wb");

	if (pOutputFile == nullptr)
	{
		free(pData);
		return;
	}

	fwrite(pData, 1, cacheDataSize, pOutputFile);

	fclose(pOutputFile);

	free(pData);
}

void MyVulkan::CreateDescriptorSetLayout()
{
	VkDescriptorSetLayoutCreateInfo createInfo;
	VkDescriptorSetLayout layout;
	vkCreateDescriptorSetLayout(logicalDevices.front(),
		&createInfo,
		&myAllocator,
		&layout);
}

void MyVulkan::CreatePipelineLayout()
{
	// This describes our combined image-samplers.
	// One set, two disjoint binding

	static const VkDescriptorSetLayoutBinding samplers[] =
	{
		{
			0,																										// Start from binding 0
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,		// Combined image-sampler
			1, 																									// Create One binding
			VK_SHADER_STAGE_ALL,															// Usable in all stages
			nullptr																								// No static samplers
		},

		{
			2,																										// Start from binding 2
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,		// Combined image-sampler
			1,																										// Create one binding
			VK_SHADER_STAGE_ALL,															// Usable in all stages
			nullptr																								// No static samplers
		}
	};

	static const VkDescriptorSetLayoutBinding uniformBlock =
	{
		0, 																										// Start from binding 0
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,								// Uniform block
		1,																											// One binding
		VK_SHADER_STAGE_ALL,																// All stages
		nullptr																									// No static samplers
	};

	// Now create the two descriptor set layouts
	static const VkDescriptorSetLayoutCreateInfo createInfoSamplers =
	{
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		2,
		&samplers[0]
	};

	static const VkDescriptorSetLayoutCreateInfo createInfoUniforms =
	{
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		1,
		&uniformBlock
	};

	// This array holds the two set layouts
	VkDescriptorSetLayout setLayouts[2];
	vkCreateDescriptorSetLayout(
		device,
		&createInfoSamplers,
		nullptr,
		&setLayouts[0]
	);
	vkCreateDescriptorSetLayout(
		device,
		&createInfoUniforms,
		nullptr,
		&setLayouts[1]
	);

	// Now create the pipeline layout.
	const VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo =
	{
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,		// sType
		nullptr,																									// pNext
		0, 																										// flags
		2,																											// setLayoutCount
		setLayouts,																							// pSetLayouts
		0,																											// pushConstantRangeCount
		nullptr																									// pPushConstantRanges
	};

	VkPipelineLayout pipelineLayout;

	vkCreatePipelineLayout(
		device,
		&pipelineLayoutCreateInfo,
		nullptr,
		pipelineLayout
		)
}
*/
/*
void MyVulkan::CreateSimpleRenderpass()
{
	// This is our color attachment. It's an R8G8B8A8_UNORM single sample image.
	// We want to clear it at the start of the renderpass and save the contents when we are done.
	// It starts in UNDEFINED layout, which is a key to Vulkan that it is allowed to throw the old content away,
	// and we want to leave it in COLOR_ATTACHMENT_OPTIMAL state when we are done.	

	static const VkAttachmentDescription attachments[] =
	{
		{
			0,																										// flags
			VK_FORMAT_R8G8B8A8_UNORM,												// format
			VK_SAMPLE_COUNT_1_BIT,														// samples
			VK_ATTACHMENT_LOAD_OP_CLEAR,										// loadOp
			VK_ATTACHMENT_STORE_OP_STORE,										// storeOp
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,								// stencilLoadOp
			VK_ATTACHMENT_STORE_OP_DONT_CARE,							// stencilStoreOp
			VK_IMAGE_LAYOUT_UNDEFINED,												// initialLayout
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL			// finalLayout
		}
	};

	// This is the single reference to our single attachment.
	static const VkAttachmentReference attachmentReferences[] =
	{
		{
			0,																									// attachment
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL		// layout
		}
	};

	// There is one subpass in this renderpass, with only a reference to the single output attachment.
	static const VkSubpassDescription subpasses[] =
	{
		{
			0,																		// flags
			VK_PIPELINE_BIND_POINT_GRAPHICS,	// pipelineBindPoint
			0,																		// inputAttachmentCount
			nullptr,																// pInputAttachments
			1,																		// colorAttachmentCount
			&attachmentReferences[0],								// pColorAttachments
			nullptr,																// pResolveAttachments
			nullptr,																// pDepthStencilAttachment
			0,																		// preserveAttachmentCount
			nullptr																// pPreserveAttachments
		}
	};

	// Finally, this is the information that Vulkan needs to create the render pass object.
	static VkRenderPassCreateInfo renderpassCreateInfo =
	{
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,			// sType
		nullptr,																									// pNext
		0,																											// flags
		1,																											// attachmentCount
		&attachments[0],																					// pAttachments
		1,																											// subpassCount
		&subpasses[0],																					// pSubpasses
		0,																											// dependencyCount
		nullptr																									// pDependencies
	};

	VkRenderPass renderPass = VK_NULL_HANDLE;

	// The only code that actually executes is this single call, which creates the renderpass object.
	vkCreateRenderPass(
		device,
		&renderpassCreateInfo,
		nullptr,
		&renderPass);
}*/

void MyVulkan::InitGUI()
{
	MyImGUI::InitImGUI(windowHolder->glfwWindow, device, instance, physicalDevice, queue, renderPass, commandBuffers.front());

	MyImGUI::SendModelInfo(model);
}

bool MyVulkan::CreateInstance(const char* appName, uint32_t appVersion)
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

	VkValidationFeaturesEXT bestPracticesEnabled = EnableBestPracticesValidation();

	// Create an instance
	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = &bestPracticesEnabled;
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

void MyVulkan::DestroyInstance()
{
	vkDestroyInstance(instance, nullptr);
}

void MyVulkan::CreatePhysicalDevice()
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

void MyVulkan::ChooseQueueFamily()
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

bool MyVulkan::CreateDevice()
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

void MyVulkan::DestroyDevice()
{
	vkDestroyDevice(device, nullptr);
}

void MyVulkan::GetCommandQueue()
{
	// Why queueIndex is 0???
	vkGetDeviceQueue(device, queueFamily, 0, &queue);
}

// VkSurface is Vulkan's name for the screen.
// Since GLFW creates and manges the window, it creates the VkSurface at our request.
bool MyVulkan::CreateSurfaceByGLFW()
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

void MyVulkan::DestroySurface()
{
	vkDestroySurfaceKHR(instance, surface, nullptr);
}

// Create a command pool used to allocate command buffers, which in turn used to gather and send commands to the GPU.
// The flag makes it possible to reuse command buffers.
// The queue index determines which queue the command buffers can be submitted to.
// Use the command pool to also create a command buffer.
bool MyVulkan::CreateCommandPoolAndAllocateCommandBuffers()
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

void MyVulkan::DestroyCommandPool()
{
	vkDestroyCommandPool(device, commandPool, nullptr);
}

bool MyVulkan::CreateSwapchain()
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

const VkSurfaceFormatKHR& MyVulkan::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) const
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

const VkPresentModeKHR MyVulkan::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& presentModes) const
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

const VkExtent2D MyVulkan::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const
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

void MyVulkan::DestroySwapchain()
{
	vkDestroySwapchainKHR(device, swapchain, nullptr);
}

void MyVulkan::GetSwapchainImages()
{
	uint32_t imageCount;
	VulkanHelper::VkCheck(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr), "Getting number of swapchain images has failed!");
	swapchainImages.resize(imageCount);
	VulkanHelper::VkCheck(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data()), "Get swapchain images has failed!");
}

void MyVulkan::CreateImageViews()
{
	swapchainImageViews.resize(swapchainImages.size());

	const size_t swapchainImageCount = swapchainImages.size();
	for (size_t i = 0; i < swapchainImageCount; i++)
	{
		swapchainImageViews[i] = CreateImageView(swapchainImages[i], swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

void MyVulkan::DestroyImageViews()
{
	for (VkImageView& imageView : swapchainImageViews)
	{
		vkDestroyImageView(device, imageView, nullptr);
	}
}

void MyVulkan::CreateRenderPass()
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
	depthAttachment.format = FindDepthFormat();
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

	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
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

void MyVulkan::CreateGraphicsPipeline()
{
	VkShaderModule vertModule = CreateShaderModule(readFile("spv/vertexShader.vert.spv"));
	VkShaderModule fragModule = CreateShaderModule(readFile("spv/fragShader.frag.spv"));

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertModule;
	vertShaderStageInfo.pName = "main";
	// pSpecializationInfo which is an optional allows you to specify values for shader constants.

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};
	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
	dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	const VkVertexInputBindingDescription& bindingDescription = Vertex::GetBindingDescription();
	const std::vector<VkVertexInputAttributeDescription>& attributeDescription = Vertex::GetAttributeDescriptions();
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
	inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	// If it is true, then fragments that are beyond the near and far planes are clamped to them as opposed to discarding them.
	// It is useful in some special cases like shadow maps which requires enabling a GPU feature.
	rasterizer.depthClampEnable = VK_FALSE;
	// If it is true, then geometry never passes through the rasterizer stage.
	// It is basically disables any output to the framebuffer.
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	// It determines how fragments are generated for geometry.
	// Using VK_POLYGON_MODE_LINE, MODE_POINT requires enabling a GPU feature.
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	// Any line thicker than 1.f requires you to enable the wideLines GPU feature.
	rasterizer.lineWidth = 1.f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.f;
	depthStencil.maxDepthBounds = 1.f;
	depthStencil.stencilTestEnable = VK_FALSE;

	/*
	Bellow blending options would be this.
	
	finalColor.rgb = newAlpha * newColor + (1 - newAlpha) * oldColor;
	finalColor.a = newAlpha.a;
	*/
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	// Bellow settings are optional, unless blendEnable is VK_FALSE.
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	VulkanHelper::VkCheck(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout), "Creating pipelineLayout has failed!");

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	// shader stages
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	// Fixed-function states
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicStateCreateInfo;
	// Pipeline layout
	pipelineInfo.layout = pipelineLayout;
	// Render pass
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	// Below parameters are used for specify parent pipeline to handle derived multiple pipelines, which we don't use it here.
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	VulkanHelper::VkCheck(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline), "Creating graphics pipeline has failed!");



	vkDestroyShaderModule(device, vertModule, nullptr);
	vkDestroyShaderModule(device, fragModule, nullptr);
}

std::vector<char> MyVulkan::readFile(const std::string& filename)
{
	// ate -> read from at the end.
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		VulkanHelper::VkCheck(VK_ERROR_UNKNOWN, std::string(std::string("Opening spv file has failed!\nFilename: ") + filename).c_str());
	}

	// tellg -> return position of the current character
	//				-> Since we opened it with ate, current position would be size of the file.
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

VkShaderModule MyVulkan::CreateShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
	
	VkShaderModule shaderModule;
	VulkanHelper::VkCheck(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule), "Creating shader module has failed!");

	return shaderModule;
}

void MyVulkan::DestroyPipeline()
{
	vkDestroyPipeline(device, pipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);
}

void MyVulkan::CreateFramebuffers()
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

void MyVulkan::DestroyFramebuffers()
{
	for (VkFramebuffer& framebuffer : swapchainFramebuffers)
	{
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}
}

void MyVulkan::CreateSyncObjects()
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

void MyVulkan::DestroySyncObjects()
{
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}
}

void MyVulkan::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;

	VulkanHelper::VkCheck(vkBeginCommandBuffer(commandBuffer, &beginInfo), "Begining command buffer has failed!");


	// Starting a render pass??????
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.framebuffer = swapchainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = swapchainExtent;

	// Since VkClearValue is union, use appropriate member variable name for usage.
	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { {0.f, 0.f, 0.f, 1.f} };
	clearValues[1].depthStencil = { 1.f, 0 };

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	VkBuffer vertexBuffers[] = { vertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrameID], 0, nullptr);

	VkViewport viewport{};
	viewport.x = 0.f;
	viewport.y = 0.f;
	viewport.width = static_cast<float>(swapchainExtent.width);
	viewport.height = static_cast<float>(swapchainExtent.height);
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapchainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);

	MyImGUI::GUIRender(commandBuffer);

	vkCmdEndRenderPass(commandBuffer);

	VulkanHelper::VkCheck(vkEndCommandBuffer(commandBuffer), "Ending command buffer has failed!");
}

void MyVulkan::RecordClientData()
{
	std::ofstream resultFile;
	void* getHowMany = nullptr;

	// Get Physical devices
	uint32_t numOfPhysicalDevices = 0;
	std::vector<VkPhysicalDevice> arrayOfPhysicalDevices;
	// second parameter of VK physical devices function works both input & output.
	// As an output, the parameter get how many physical devices I can use.
	// As an input, The maximum number of devices I can control in this application.
	// Once we want to know how many devices available in the system, give final parameter nullptr.(second parameter still should be valid pointer)
	// Then call the same function again with the final parameter set to an array that has been appropriately sized for the number what we have known.
	VulkanHelper::VkCheck(vkEnumeratePhysicalDevices(instance, &numOfPhysicalDevices, reinterpret_cast<VkPhysicalDevice*>(getHowMany)), "The first Procedure with physical devices is failed! (Originally, it might be failed)");
	arrayOfPhysicalDevices.resize(numOfPhysicalDevices);
	VulkanHelper::VkCheck(vkEnumeratePhysicalDevices(instance, &numOfPhysicalDevices, arrayOfPhysicalDevices.data()), "The second Procedure with physical devices is failed! (Logically, should not be failed)");

	// Create logical devices per physical device
	for (const VkPhysicalDevice& physicalDevice : arrayOfPhysicalDevices)
	{

		VkPhysicalDeviceProperties physicalProperty{};
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalProperty);
		/// PRINT
		resultFile.open("Result of " + std::string(physicalProperty.deviceName) + ".txt");																   /// PRINT
		resultFile << "apiVersion: " << physicalProperty.apiVersion << std::endl;																   /// PRINT
		resultFile << "driverVersion: " << physicalProperty.driverVersion << std::endl;															 /// PRINT
		resultFile << "vendorID: " << physicalProperty.vendorID << std::endl;																		/// PRINT
		resultFile << "deviceID: " << physicalProperty.deviceID << std::endl;																		 /// PRINT
		resultFile << "deviceType: " << physicalProperty.deviceType << std::endl;																 /// PRINT
		resultFile << "deviceName: " << physicalProperty.deviceName << std::endl << std::endl << std::endl;					   /// PRINT




		// First determine the number of queue families supported by the physical device.
		uint32_t numOfQueueFamilyProperty = 0;
		std::vector<VkQueueFamilyProperties> familyProperties;
		// Bellow code works similar with vkEnumeratePhysicalDevice()
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numOfQueueFamilyProperty, reinterpret_cast<VkQueueFamilyProperties*>(getHowMany));
		// Allocate enough space for the queue property structures.
		familyProperties.resize(numOfQueueFamilyProperty);
		// Now query the actual properties of the queue families.
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numOfQueueFamilyProperty, familyProperties.data());

		for (const VkQueueFamilyProperties& familyProperty : familyProperties)
		{
			resultFile << "Queue Family Count: " << numOfQueueFamilyProperty << std::endl;																							 /// PRINT
			resultFile << "queueFlags: " << familyProperty.queueFlags << std::endl;																							   /// PRINT
			resultFile << "queueCount: " << familyProperty.queueCount << std::endl;																							  /// PRINT
			resultFile << "timestampValidBits: " << familyProperty.timestampValidBits << std::endl;																		/// PRINT
			resultFile << "minImageTransferGranularity.width: " << familyProperty.minImageTransferGranularity.width << std::endl;					   /// PRINT
			resultFile << "minImageTransferGranularity.height: " << familyProperty.minImageTransferGranularity.height << std::endl;				  /// PRINT
			resultFile << "minImageTransferGranularity.depth: " << familyProperty.minImageTransferGranularity.depth << std::endl << std::endl << std::endl;					  /// PRINT
		}

		// Create Logical Device
		VkDeviceQueueCreateInfo logicalDeviceQueueCreateInfo{};
		logicalDeviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		logicalDeviceQueueCreateInfo.pNext = nullptr;
		logicalDeviceQueueCreateInfo.flags = 0;
		logicalDeviceQueueCreateInfo.queueFamilyIndex = 0;		// ????????? I do not know what this field is for....
		logicalDeviceQueueCreateInfo.queueCount = 1;
		logicalDeviceQueueCreateInfo.pQueuePriorities = nullptr;

		// Get Layers
		uint32_t layerCount = 0;
		std::vector<VkLayerProperties> layers;
		std::vector<const char*> layerNames;
		// Query the instance layers
		VulkanHelper::VkCheck(vkEnumerateInstanceLayerProperties(&layerCount, reinterpret_cast<VkLayerProperties*>(getHowMany)), "First Layer : Layer enumeration is failed! when pointer to array is nullptr");

		resultFile << "layerCount: " << layerCount << std::endl << std::endl;		/// PRINT
		if (layerCount != 0)
		{
			layers.resize(layerCount);
			VulkanHelper::VkCheck(vkEnumerateInstanceLayerProperties(&layerCount, layers.data()), "Second Layer : Layer enumeration is failed! when pointer to array is not nullptr");
			for (const VkLayerProperties& layer : layers)
			{

				resultFile << "layerName: " << layer.layerName << std::endl;		/// PRINT
				resultFile << "specVersion: " << layer.specVersion << std::endl;		/// PRINT
				resultFile << "implementationVersion: " << layer.implementationVersion << std::endl;		/// PRINT
				resultFile << "description: " << layer.description << std::endl << std::endl;		/// PRINT
				layerNames.push_back(layer.layerName);
			}
		}
		else
		{
			std::cout << "WARNING:: available layer is zero!" << std::endl;
		}

		resultFile << std::endl << std::endl;

		// Get Extensions
		/*!!!!!!!!!!!!!!!!!!!!!!!!!!!! However, since extension need cost, for now do not use extension. Let us prefer vanilla mode*/
		uint32_t instanceExtensionCount = 0;
		std::vector<VkExtensionProperties> instanceExtensions;
		std::vector<const char*> extensionNames;
		VulkanHelper::VkCheck(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, reinterpret_cast<VkExtensionProperties*>(getHowMany)), "Error during get instance extesions!");

		resultFile << "extensionCount: " << instanceExtensionCount << std::endl << std::endl;		/// PRINT
		if (instanceExtensionCount > 0)
		{
			instanceExtensions.resize(instanceExtensionCount);
			VulkanHelper::VkCheck(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, instanceExtensions.data()), "Error during get instance extesions!");
			for (VkExtensionProperties extension : instanceExtensions)
			{
				resultFile << "extensionName: " << extension.extensionName << std::endl;		/// PRINT
				resultFile << "specVersion: " << extension.specVersion << std::endl << std::endl;		/// PRINT
				extensionNames.push_back(extension.extensionName);
			}
		}
		else
		{
			std::cout << "Supported extensions are zero" << std::endl;
		}

		VkPhysicalDeviceFeatures supportedFeatures{};
		vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);
		VkPhysicalDeviceFeatures requiredFeatures{};
		// Set features based on book's example
		requiredFeatures.multiDrawIndirect = supportedFeatures.multiDrawIndirect;
		requiredFeatures.tessellationShader = VK_TRUE;
		requiredFeatures.geometryShader = VK_TRUE;

		VkDeviceCreateInfo logicalDeviceCreateInfo{};
		logicalDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		logicalDeviceCreateInfo.pNext = 0;
		logicalDeviceCreateInfo.flags = 0;
		// For this time, let this value one. (At this moment, I'm not sure I can control two or more queues)
		logicalDeviceCreateInfo.queueCreateInfoCount = 1;
		logicalDeviceCreateInfo.pQueueCreateInfos = &logicalDeviceQueueCreateInfo;
		// We are going to cover layer and extension later in this chapter.
		logicalDeviceCreateInfo.enabledLayerCount = layerCount;
		logicalDeviceCreateInfo.ppEnabledLayerNames = layerNames.data();
		logicalDeviceCreateInfo.enabledExtensionCount = 0;
		logicalDeviceCreateInfo.ppEnabledExtensionNames = nullptr;
		logicalDeviceCreateInfo.pEnabledFeatures = &requiredFeatures;

		VkDevice logicalDevice{};

		resultFile.close();
	}
}

void MyVulkan::UpdateCurrentFrameID()
{
	currentFrameID = (currentFrameID + 1) % MAX_FRAMES_IN_FLIGHT;
}

void MyVulkan::RecreateSwapchain()
{
	int width = 0;
	int height = 0;
	glfwGetFramebufferSize(windowHolder->glfwWindow, &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(windowHolder->glfwWindow, &width, &height);
		glfwWaitEvents();
	}


	vkDeviceWaitIdle(device);

	CleanupSwapchainResourcesForRecreation();

	CreateSwapchain();
	CreateImageViews();
	CreateDepthResources();
	CreateFramebuffers();
}

void MyVulkan::CleanupSwapchainResourcesForRecreation()
{
	DestroyFramebuffers();
	DestroyImageViews();
	DestroyDepthResources();
	DestroySwapchain();
}

void MyVulkan::CreateVertexBuffer(int vertexCount, void* vertexData)
{
	// Use two buffers.
	// One for writing vertex data, the other is actual vertex buffer which we cannot see and use(map) at CPU.
	// The reason why use two buffers is the buffer we can see at CPU is not a good buffer from the GPU side.
	VkDeviceSize bufferSize = sizeof(Vertex) * vertexCount;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	// Operate as glMapBuffer, glUnmapBuffer
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertexData, static_cast<size_t>(bufferSize));
	vkUnmapMemory(device, stagingBufferMemory);

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

	CopyBuffer(stagingBuffer, vertexBuffer, bufferSize);

	DestroyBuffer(stagingBuffer);
	FreeMemory(stagingBufferMemory);
}

void MyVulkan::DestroyBuffersAndFreeMemories()
{
	DestroyModelBuffers();

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		DestroyBuffer(uniformBuffers[i]);
		FreeMemory(uniformBuffersMemory[i]);
	}
}

void MyVulkan::CreateModelBuffers()
{
	CreateVertexBuffer(model->GetVertexCount(), model->GetVertexData());
	CreateIndexBuffer(model->GetIndexCount(), model->GetIndexData());
}

void MyVulkan::DestroyModelBuffers()
{
	DestroyBuffer(vertexBuffer);
	FreeMemory(vertexBufferMemory);

	DestroyBuffer(indexBuffer);
	FreeMemory(indexBufferMemory);
}

void MyVulkan::DestroyBuffer(VkBuffer& buffer)
{
	vkDestroyBuffer(device, buffer, nullptr);
}

void MyVulkan::FreeMemory(VkDeviceMemory memory)
{
	vkFreeMemory(device, memory, nullptr);
}


uint32_t MyVulkan::FindMemoryTypeIndex(const VkMemoryRequirements& memoryRequirements)
{
	for (uint32_t memoryType = 0; memoryType < VK_MAX_MEMORY_TYPES; memoryType++)
	{
		if ((memoryRequirements.memoryTypeBits & memoryType) != 0)
		{
			return memoryType;
		}
	}

	std::cout << "Failed to find suitable memory type!" << std::endl;
	return ~0U;
}

uint32_t MyVulkan::FindMemoryTypeIndex(const VkMemoryRequirements& memoryRequirements, VkMemoryPropertyFlags requiredFlags)
{
	VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);

	for (uint32_t memoryType = 0; memoryType < VK_MAX_MEMORY_TYPES; memoryType++)
	{
		if (memoryRequirements.memoryTypeBits & (1 << memoryType))
		{
			const VkMemoryType& type = deviceMemoryProperties.memoryTypes[memoryType];

			// If it has all my required properties, it'll do.
			if ((type.propertyFlags & requiredFlags) == requiredFlags)
			{
				return memoryType;
			}
		}
	}

	std::cout << "Failed to find suitable memory type!" << std::endl;
	return ~0U;
}

void MyVulkan::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
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
	allocInfo.memoryTypeIndex = FindMemoryTypeIndex(memRequirements, properties);

	VulkanHelper::VkCheck(vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory), "Allocating vertex buffer memory has failed!");
	vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void MyVulkan::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer copyCommandBuffer = BeginSingleTimeCommands();

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(copyCommandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	EndSingleTimeCommands(copyCommandBuffer);
}

void MyVulkan::CreateIndexBuffer(int indexCount, void* indexData)
{
	this->indexCount = indexCount;
	VkDeviceSize bufferSize = sizeof(int32_t) * indexCount;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indexData, bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		indexBuffer, indexBufferMemory);

	CopyBuffer(stagingBuffer, indexBuffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void MyVulkan::CreateDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr;		// Optional

	//VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	//samplerLayoutBinding.binding = 1;
	//samplerLayoutBinding.descriptorCount = 1;
	//samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	//samplerLayoutBinding.pImmutableSamplers = nullptr;
	//samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, /*2*/1> bindings = { uboLayoutBinding/*, samplerLayoutBinding */};

	VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutCreateInfo.pBindings = bindings.data();

	VulkanHelper::VkCheck(vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &descriptorSetLayout), "Creating descriptor set layout has failed!");
}

void MyVulkan::DestroyDescriptorSetLayout()
{
	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
}

void MyVulkan::CreateUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);
	uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			uniformBuffers[i], uniformBuffersMemory[i]);
	}
}

void MyVulkan::UpdateUniformBuffer(uint32_t currentImage)
{
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	UniformBufferObject ubo;
	ubo.model = glm::rotate(glm::mat4(1.f), time * glm::radians(90.f), glm::vec3(0.0f, 0.0f, 1.0f)) * 
		glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(1.0f, 0.0f, 0.0f)) * 
		model->CalculateAdjustBoundingBoxMatrix();
	ubo.view = glm::lookAt(glm::vec3(2.f, 2.f, 2.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
	ubo.proj = glm::perspective(glm::radians(45.f), swapchainExtent.width / static_cast<float>(swapchainExtent.height), 0.1f, 10.f);
	// flip the sign of the element because GLM originally designed for OpenGL, where Y coordinate of the clip coorinates is inverted.
	ubo.proj[1][1] *= -1;

	void* data;
	vkMapMemory(device, uniformBuffersMemory[currentFrameID], 0, sizeof(UniformBufferObject), 0, &data);
	memcpy(data, &ubo, sizeof(UniformBufferObject));
	vkUnmapMemory(device, uniformBuffersMemory[currentFrameID]);
}

void MyVulkan::CreateDescriptorPool()
{
	std::array<VkDescriptorPoolSize, /*2*/1> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	//poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	//poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	// maxSets is the maximum number of descriptor sets that can be allocated from the pool
	poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	VulkanHelper::VkCheck(vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool), "Creating descriptor pool has failed!");
}

void MyVulkan::DestroyDescriptorPool()
{
	vkDestroyDescriptorPool(device, descriptorPool, nullptr);
}

void MyVulkan::CreateDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
	// vkAllocateDescriptorSets may fail with the error code VK_ERROR_POOL_OUT_OF_MEMORY 
		// if the pool is not sufficiently large, 
		// but the driver may also try to solve the problem internally.

	VulkanHelper::VkCheck(vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()), "Allocating descriptor sets has failed!");

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		//VkDescriptorImageInfo imageInfo{};
		//imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		//// @@ TODO: Multiple textures? or single textures?
		//imageInfo.imageView = textureImageViews[0];
		//imageInfo.sampler = textureSampler;

		std::vector<VkWriteDescriptorSet> descriptorWrites(/*2*/1);

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		// The starting element in that array
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		//descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		//descriptorWrites[1].dstSet = descriptorSets[i];
		//descriptorWrites[1].dstBinding = 1;
		//descriptorWrites[1].dstArrayElement = 0;
		//descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		//descriptorWrites[1].descriptorCount = 1;
		//descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void MyVulkan::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
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

void MyVulkan::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
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

uint32_t MyVulkan::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	std::cout << "Finding suitable memory type has failed!" << std::endl;
	abort();
}

VkImageView MyVulkan::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
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

void MyVulkan::CreateTextureSampler()
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

void MyVulkan::DestroyTextureImage()
{
	size_t imageCount = textureImages.size();
	for (size_t i = 0; i < imageCount; i++)
	{
		vkDestroyImageView(device, textureImageViews[i], nullptr);
		vkDestroyImage(device, textureImages[i], nullptr);
		vkFreeMemory(device, textureImageMemories[i], nullptr);
	}
}

void MyVulkan::DestroyTextureSampler()
{
	vkDestroySampler(device, textureSampler, nullptr);
}

VkValidationFeaturesEXT MyVulkan::EnableBestPracticesValidation()
{
	VkValidationFeatureEnableEXT enables[] = { VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT };
	VkValidationFeaturesEXT features{};
	features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
	features.enabledValidationFeatureCount = 1;
	features.pEnabledValidationFeatures = enables;

	return features;
}

void MyVulkan::CreateDepthResources()
{
	VkFormat depthFormat = FindDepthFormat();

	CreateImage(swapchainExtent.width, swapchainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
	depthImageView = CreateImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void MyVulkan::DestroyDepthResources()
{
	vkDestroyImageView(device, depthImageView, nullptr);
	vkDestroyImage(device, depthImage, nullptr);
	vkFreeMemory(device, depthImageMemory, nullptr);
}

VkFormat MyVulkan::FindDepthFormat()
{
	return FindSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

bool MyVulkan::HasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkFormat MyVulkan::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties prop;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &prop);

		if (tiling == VK_IMAGE_TILING_LINEAR && (prop.linearTilingFeatures & features) == features)
		{
			return format;
		}
		else if(tiling == VK_IMAGE_TILING_OPTIMAL && (prop.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}

	// Should change to better exception checks
	std::cout << "There are no supported format!" << std::endl;
	abort();
}

VkCommandBuffer MyVulkan::BeginSingleTimeCommands()
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

void MyVulkan::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
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

void MyVulkan::CreateTextures(const std::vector<std::string>& imagePaths)
{
	size_t imageCount = imagePaths.size();

	for (size_t i = 0; i < imageCount; i++)
	{
		CreateTextureImageAndImageView(imagePaths[i]);
	}
}

void MyVulkan::CreateTextureImageAndImageView(const std::string& path)
{
	int texWidth, texHeight, texChannels;

	stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	constexpr int RGBA = 4;
	VkDeviceSize imageSize = texWidth * texHeight * RGBA;

	if (!pixels)
	{
		windowHolder->DisplayMessage("Texture Error", "Loading texture image has Failed!");
	}

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(device, stagingBufferMemory);

	stbi_image_free(pixels);

	VkImage textureImage;
	VkDeviceMemory textureImageMemory;

	CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
	
	TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	CopyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
	// Finished creating texture image (create staging buffer, copy image data, copy buffer data to image buffer, clean staging buffer)

	// Create Image View
	VkImageView textureImageView = CreateImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

	textureImages.push_back(textureImage);
	textureImageMemories.push_back(textureImageMemory);
	textureImageViews.push_back(textureImageView);
}
void MyVulkan::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
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
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

	VulkanHelper::VkCheck(vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory), "Allocating memory has failed!");

	// Note that using vkBindImageMemoryinstead of vkBindBufferMemory
	vkBindImageMemory(device, image, imageMemory, 0);
}

/*
void MyVulkan::DescribeVertexInputData()
{
	typedef struct vertex_t
	{
		glm::vec4 position;
		glm::vec3 normal;
		glm::vec2 texcoord;
	} vertex;

	static const VkVertexInputBindingDescription vertexInputBindings[] =
	{
		{ 0, sizeof(vertex), VK_VERTEX_INPUT_RATE_VERTEX }		// Buffer
	};

	// need to improve to use comfortably
	static const VkVertexInputAttributeDescription vertexAttributes[] =
	{
		{ 0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0 },																	// position
		{ 1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(vertex, normal) },								// normal
		{ 2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(vertex, texcoord) }								// texcoord
	};

	static const VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo =
	{
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		nullptr,
		0,
		RTL_NUMBER_OF_V2(vertexInputBindings),
		vertexInputBindings,
		RTL_NUMBER_OF_V2(vertexAttributes),
		vertexAttributes
	};
}

void MyVulkan::SetupSeparateVertexAttribute()
{
	typedef struct vertex_t
	{
		glm::vec3 normal;
		glm::vec2 texcoord;
	} vertex;

	static const VkVertexInputBindingDescription vertexInputBindings[] =
	{
		{0, sizeof(glm::vec4), VK_VERTEX_INPUT_RATE_VERTEX},		// Buffer 1
		{1, sizeof(vertex), VK_VERTEX_INPUT_RATE_VERTEX}				// Buffer 2
	};

	static const VkVertexInputAttributeDescription vertexAttributes[] =
	{
		{ 0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0 },					// Position
		{ 1, 1, VK_FORMAT_R32G32B32_SFLOAT, 0 },						// Normal
		{ 2, 1, VK_FORMAT_R32G32_SFLOAT, sizeof(glm::vec3) }		// Tex Coord
	};

	static const VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo =
	{
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		nullptr,
		0,
		RTL_NUMBER_OF_V2(vertexInputBindings),
		vertexInputBindings,
		RTL_NUMBER_OF_V2(vertexAttributes),
		vertexAttributes
	};

}

*/