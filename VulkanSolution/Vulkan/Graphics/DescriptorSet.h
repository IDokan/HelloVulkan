/******************************************************************************
Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   DescriptorSet.h
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date 11.05.2022
	header file for descriptor set wrapper.
******************************************************************************/
#pragma once

#include <vector>
#include <vulkan/vulkan.h>

class DescriptorSet
{
public:
	DescriptorSet(unsigned int descriptorSetSize, VkDevice device, std::vector<VkDescriptorSetLayoutBinding> layoutBindings);
	~DescriptorSet();

	void Write(size_t descriptorIndex, uint32_t dstBinding, const VkBuffer& buffer, VkDeviceSize range);
	void Write(size_t descriptorIndex, uint32_t dstBinding, const VkImageView& imageView, const VkSampler& sampler);
private:
	void CreateDescriptorPoolSize(std::vector<VkDescriptorPoolSize>& poolSizes);

	unsigned int descriptorSetSize;

	VkDevice device;
	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	std::vector<VkDescriptorSet> descriptorSets;

	std::vector<VkDescriptorSetLayoutBinding> bindingTable;
};