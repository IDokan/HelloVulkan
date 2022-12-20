/******************************************************************************
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   VulkanHelper.h
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 06.12.2021
	header file for vulkan helper.
		- result checks
******************************************************************************/
#pragma once
#include <iostream>

enum VkResult;

namespace VulkanHelper
{
	VkResult VkCheck(VkResult result, const char* errorMsg);


	inline VkResult VulkanHelper::VkCheck(VkResult result, const char* errorMsg)
	{
#ifdef _DEBUG
		if (result != VK_SUCCESS)
		{
			std::cout << errorMsg << std::endl;
			abort();
		}
#endif
		return result;
	}


	VkFormat FindDepthFormat(VkPhysicalDevice physicalDevice);



	VkFormat FindDepthFormat(VkPhysicalDevice physicalDevice)
	{
		return FindSupportedFormat(physicalDevice, { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}

	VkFormat FindSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);



	VkFormat FindSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties prop;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &prop);

			if (tiling == VK_IMAGE_TILING_LINEAR && (prop.linearTilingFeatures & features) == features)
			{
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (prop.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}

		// Should change to better exception checks
		std::cout << "There are no supported format!" << std::endl;
		abort();
	}

	uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
	uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
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



	// When requiredFlags are not,
	uint32_t FindMemoryTypeIndex(const VkMemoryRequirements& memoryRequirements)
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

	// returns ~0u if matched memory does not exist.
	uint32_t FindMemoryTypeIndex(VkPhysicalDevice physicalDevice, const VkMemoryRequirements& memoryRequirements, VkMemoryPropertyFlags requiredFlags)
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

// #define ARRAYSIZE(ARR) sizeof(ARR) / sizeof(ARR ## [0])
}