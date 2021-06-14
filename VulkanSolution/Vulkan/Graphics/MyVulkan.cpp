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
	std::vector<VkDevice> logicalDevices;
	allocator allocatorForm;
	const VkAllocationCallbacks myAllocator = (VkAllocationCallbacks)allocatorForm;

	double linearTosRGB(double cl);
	double sRGBToLinear(double cs);
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