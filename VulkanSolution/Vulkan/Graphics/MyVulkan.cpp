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
#include <iostream>
#include <fstream>
#include <vector>
#include "Graphics/MyVulkan.h"
#include "Vulkan/vulkan.h"
#include "Helper/VulkanHelper.h"
#include "Graphics/Allocator/Allocator.h"

namespace MyVulkan
{
	VkInstance instance{};
	std::vector<VkPhysicalDevice> physicalDevices;
	std::vector<VkDevice> logicalDevices;
	allocator allocatorForm;
	const VkAllocationCallbacks myAllocator = (VkAllocationCallbacks)allocatorForm;

	double linearTosRGB(double cl);
	double sRGBToLinear(double cs);

	// When required or preferredFlags are not,
	uint32_t ChooseHeapFromFlags(
		const VkMemoryRequirements& memoryRequirements);
	// returns ~0u if matched memory does not exist.
	uint32_t ChooseHeapFromFlags(
		const VkMemoryRequirements& memoryRequirements,
		VkMemoryPropertyFlags requiredFlags,
		VkMemoryPropertyFlags preferredFlags);

	void FillBufferWithFloats(VkCommandBuffer cmdBuffer, VkBuffer dstBuffer, VkDeviceSize offset, VkDeviceSize length, const float value);

	void CreateSwapChain();

	void TransitionImageLayout();

	void SavingPipelineCacheDataToFile(VkDevice device, VkPipelineCache cache, const char* fileName);

	void CreateDescriptorSetLayout();

	void CreatePipelineLayout();

}

void MyVulkan::InitVulkan(const char* appName, uint32_t appVersion)
{
	std::ofstream resultFile;

	//A generic application info structure
	VkApplicationInfo applicationInfo{};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pNext = nullptr;
	applicationInfo.pApplicationName = appName;
	applicationInfo.applicationVersion = appVersion;
	applicationInfo.pEngineName = "Sinil Engine";
	applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.apiVersion = VK_API_VERSION_1_0;
	// application.Info.apiVersion = ; it contains the version of Vulkan API that my application is expecting to run on. This should be set to the absolute minimum version


	// Create an instance
	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = nullptr;
	instanceCreateInfo.flags = 0;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;
	VulkanHelper::VkCheck(vkCreateInstance(&instanceCreateInfo, &myAllocator, &instance), "Could not create instance");


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
		physicalDevices.push_back(physicalDevice);

		/*!!!!!!!! Skip memory stuff at this moment*/
		VkPhysicalDeviceProperties physicalProperty{};
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalProperty);
#ifdef PRINT_RESULT
		/// PRINT
		resultFile.open("Result of " + std::string(physicalProperty.deviceName) + ".txt");																   /// PRINT
		resultFile << "apiVersion: " << physicalProperty.apiVersion << std::endl;																   /// PRINT
		resultFile << "driverVersion: " << physicalProperty.driverVersion << std::endl;															 /// PRINT
		resultFile << "vendorID: " << physicalProperty.vendorID << std::endl;																		/// PRINT
		resultFile << "deviceID: " << physicalProperty.deviceID << std::endl;																		 /// PRINT
		resultFile << "deviceType: " << physicalProperty.deviceType << std::endl;																 /// PRINT
		resultFile << "deviceName: " << physicalProperty.deviceName << std::endl << std::endl << std::endl;					   /// PRINT
#endif

		//VkPhysicalDeviceMemoryProperties memoryProperties;
		//// Get the memory properties of the physical device.
		//vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

		//// Do something about memory
		//for  (int m= 0; m < memoryProperties.memoryTypeCount; m++)
		//{
		//	for (int h = 0; h < memoryProperties.memoryHeapCount; h++)
		//	{
		//		// memoryProperties.memoryHeaps[]				....................
		//	}
		//}
		//memoryProperties.memoryTypes[0].propertyFlags;
		//memoryProperties.memoryHeaps[memoryProperties.memoryTypes[0].heapIndex];



		// First determine the number of queue families supported by the physical device.
		uint32_t numOfQueueFamilyProperty = 0;
		std::vector<VkQueueFamilyProperties> familyProperties;
		// Bellow code works similar with vkEnumeratePhysicalDevice()
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numOfQueueFamilyProperty, reinterpret_cast<VkQueueFamilyProperties*>(getHowMany));
		// Allocate enough space for the queue property structures.
		familyProperties.resize(numOfQueueFamilyProperty);
		// Now query the actual properties of the queue families.
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numOfQueueFamilyProperty, familyProperties.data());


#ifdef PRINT_RESULT
		resultFile << "Queue Family Count: " << numOfQueueFamilyProperty << std::endl;																							 /// PRINT
		resultFile << "queueFlags: " << familyProperties.front().queueFlags << std::endl;																							   /// PRINT
		resultFile << "queueCount: " << familyProperties.front().queueCount << std::endl;																							  /// PRINT
		resultFile << "timestampValidBits: " << familyProperties.front().timestampValidBits << std::endl;																		/// PRINT
		resultFile << "minImageTransferGranularity.width: " << familyProperties.front().minImageTransferGranularity.width << std::endl;					   /// PRINT
		resultFile << "minImageTransferGranularity.height: " << familyProperties.front().minImageTransferGranularity.height << std::endl;				  /// PRINT
		resultFile << "minImageTransferGranularity.depth: " << familyProperties.front().minImageTransferGranularity.depth << std::endl << std::endl << std::endl;					  /// PRINT
#endif

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

#ifdef PRINT_RESULT
		resultFile << "layerCount: " << layerCount << std::endl << std::endl;		/// PRINT
#endif
		if (layerCount != 0)
		{
			layers.resize(layerCount);
			VulkanHelper::VkCheck(vkEnumerateInstanceLayerProperties(&layerCount, layers.data()), "Second Layer : Layer enumeration is failed! when pointer to array is not nullptr");
			for (const VkLayerProperties& layer : layers)
			{

#ifdef PRINT_RESULT
				resultFile << "layerName: " << layer.layerName << std::endl;		/// PRINT
				resultFile << "specVersion: " << layer.specVersion << std::endl;		/// PRINT
				resultFile << "implementationVersion: " << layer.implementationVersion << std::endl;		/// PRINT
				resultFile << "description: " << layer.description << std::endl << std::endl;		/// PRINT
#endif
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

#ifdef PRINT_RESULT
		resultFile << "extensionCount: " << instanceExtensionCount << std::endl << std::endl;		/// PRINT
#endif
		if (instanceExtensionCount > 0)
		{
			instanceExtensions.resize(instanceExtensionCount);
			VulkanHelper::VkCheck(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, instanceExtensions.data()), "Error during get instance extesions!");
			for (VkExtensionProperties extension : instanceExtensions)
			{
#ifdef PRINT_RESULT
				resultFile << "extensionName: " << extension.extensionName << std::endl;		/// PRINT
				resultFile << "specVersion: " << extension.specVersion << std::endl << std::endl;		/// PRINT
#endif
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
		VulkanHelper::VkCheck(vkCreateDevice(physicalDevice, &logicalDeviceCreateInfo, &myAllocator, &logicalDevice), "Vulkan logical device creation is failed!");
		logicalDevices.push_back(logicalDevice);

#ifdef PRINT_RESULT
		resultFile.close();
#endif
	}

	CreateBuffers();
	CreateImages();
}

void MyVulkan::CleanVulkan()
{
	for (VkDevice logicalDevice : logicalDevices)
	{
		VulkanHelper::VkCheck(vkDeviceWaitIdle(logicalDevice), "failed to make logical device idle");
		vkDestroyDevice(logicalDevice, &myAllocator);
	}

	vkDestroyInstance(instance, &myAllocator);
}

void MyVulkan::CreateBuffers()
{
	static const VkBufferCreateInfo bufferCreateInfo =
	{
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,																		// VkStructureType sType
		nullptr,																																					// const void* pNext
		0,																																							// VkBufferCreateFlags flags
		1024 * 1024,																																		// VkDeviceSize size
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,	// VkBufferUsageFlags flags
		VK_SHARING_MODE_EXCLUSIVE,																									// VkSharingMode sharingMode
		0,																																							// uint32_t queueFamilyIndexCount
		nullptr																																					// const uint32_t* pQueueFamilyIndices
	};

	VkBuffer buffer = VK_NULL_HANDLE;
	VulkanHelper::VkCheck(vkCreateBuffer(logicalDevices.front(), &bufferCreateInfo, &myAllocator, &buffer), "Creating buffer is failed!");

	vkDestroyBuffer(logicalDevices.front(), buffer, &myAllocator);
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

	VulkanHelper::VkCheck(vkCreateImage(logicalDevices.front(), &imageCreateInfo, &myAllocator, &image), "Creating Image is failed");

	vkDestroyImage(logicalDevices.front(), image, &myAllocator);
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

	VulkanHelper::VkCheck(vkCreateImage(logicalDevices.front(), &createInfo, &myAllocator, &cube), "Image creation is failed!");

	VkImageViewCreateInfo viewCreateInfo;
	viewCreateInfo;

	//vkCreateImageView(logicalDevices.front(), )

	vkDestroyImage(logicalDevices.front(), cube, &myAllocator);
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

uint32_t MyVulkan::ChooseHeapFromFlags(const VkMemoryRequirements& memoryRequirements)
{
	for (uint32_t memoryType = 0; memoryType < VK_MAX_MEMORY_TYPES; memoryType++)
	{
		if ((memoryRequirements.memoryTypeBits & memoryType) != 0)
		{
			return memoryType;
		}
	}
}

uint32_t MyVulkan::ChooseHeapFromFlags(const VkMemoryRequirements& memoryRequirements, VkMemoryPropertyFlags requiredFlags, VkMemoryPropertyFlags preferredFlags)
{
	VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevices.front(), &deviceMemoryProperties);

	for (uint32_t memoryType = 0; memoryType < VK_MAX_MEMORY_TYPES; memoryType++)
	{
		if (memoryRequirements.memoryTypeBits & (1 << memoryType))
		{
			const VkMemoryType& type = deviceMemoryProperties.memoryTypes[memoryType];

			// If it exactly matches my preffered properties, grab it.
			if ((type.propertyFlags & preferredFlags) == preferredFlags)
			{
				return memoryType;
			}
		}
	}

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

	return ~0U;
}

void MyVulkan::FillBufferWithFloats(VkCommandBuffer cmdBuffer, VkBuffer dstBuffer, VkDeviceSize offset, VkDeviceSize size, const float value)
{
	vkCmdFillBuffer(cmdBuffer, dstBuffer, offset, size, *(const uint32_t*)&value);
}

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