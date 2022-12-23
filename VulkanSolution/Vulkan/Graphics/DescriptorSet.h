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
#include <Engines/Objects/Object.h>
#include <vulkan/vulkan.h>

class Graphics;

class DescriptorSet : public Object
{
public:
	DescriptorSet(Graphics* graphics, std::string name, unsigned int descriptorSetSize, std::vector<VkDescriptorSetLayoutBinding> layoutBindings);
	~DescriptorSet();

	bool Init();
	void Update(float dt);
	void Clean();

	VkDescriptorSetLayout* GetDescriptorSetLayoutPtr();
	VkDescriptorSet* GetDescriptorSetPtr(size_t index);

	void Write(size_t descriptorIndex, uint32_t dstBinding, const VkBuffer& buffer, VkDeviceSize range);
	void Write(size_t descriptorIndex, uint32_t dstBinding, const VkImageView& imageView, const VkSampler& sampler);
private:
	void CreateDescriptorPoolSize(std::vector<VkDescriptorPoolSize>& poolSizes);

	VkDevice device;

	unsigned int descriptorSetSize;

	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	std::vector<VkDescriptorSet> descriptorSets;

	std::vector<VkDescriptorSetLayoutBinding> bindingTable;
};