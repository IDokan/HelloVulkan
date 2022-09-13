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

#include "Vulkan/vulkan.h"
#include "Graphics/Allocator/Allocator.h"

class MyVulkan
{
public:
	// Return if initialization is succeed or not
	// Use VK_MAKE_VERSION(major, minor, patch) for second parameter 'appVersion'
	void InitVulkan(const char* appName, uint32_t appVersion);
	void CleanVulkan();

	void CreateBuffers();
	void CreateImages();
	void CreateCubeImages();

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

	/*
	void CreateSwapChain();

	void TransitionImageLayout();

	void SavingPipelineCacheDataToFile(VkDevice device, VkPipelineCache cache, const char* fileName);

	void CreateDescriptorSetLayout();

	void CreatePipelineLayout();
	*/

	void CreateSimpleRenderpass();

	//void CreateSimpleGraphicsPipeline();

	void DescribeVertexInputData();

	void SetupSeparateVertexAttribute();

private:
	VkInstance instance{};
	std::vector<VkPhysicalDevice> physicalDevices;
	std::vector<VkDevice> logicalDevices;
	allocator allocatorForm;
	const VkAllocationCallbacks myAllocator = (VkAllocationCallbacks)allocatorForm;
};