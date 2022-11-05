
/******************************************************************************
Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   DescriptorSet.h
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date 11.05.2022
	Source file for descriptor set wrapper.
******************************************************************************/
#include "Graphics/DescriptorSet.h"
#include "Helper/VulkanHelper.h"

DescriptorSet::DescriptorSet(unsigned int descriptorSetSize, VkDevice device, std::vector<VkDescriptorSetLayoutBinding> layoutBindings)
	:descriptorSetSize(descriptorSetSize), device(device), descriptorPool(0), descriptorSetLayout(0), descriptorSets(descriptorSetSize), bindingTable(layoutBindings)
{
	/// @@ Create Descriptor Set Layout
	VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.bindingCount = bindingTable.size();
	layoutCreateInfo.pBindings = bindingTable.data();

	VulkanHelper::VkCheck(vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &descriptorSetLayout), "Creating descriptor set layout has failed!");
	/// @@ End of creating Descriptor Set Layout


	/// @@ Create Descriptor Pool
	// Assemble the same type descriptor set bindings to the sum of them in one descriptor type.
	std::vector<VkDescriptorPoolSize> poolSizes;
	CreateDescriptorPoolSize(poolSizes);
	
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = descriptorSetSize;

	VulkanHelper::VkCheck(vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool), "Creating descriptor pool has failed!");
	/// @@ End of creating Descriptor Pool


	/// @@ Create Descriptor Sets
	std::vector<VkDescriptorSetLayout> layouts(descriptorSetSize, descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(descriptorSetSize);
	allocInfo.pSetLayouts = layouts.data();

	// vkAllocateDescriptorSets may fail with the error code VK_ERROR_POOL_OUT_OF_MEMORY 
		// if the pool is not sufficiently large, 
		// but the driver may also try to solve the problem internally.
	VulkanHelper::VkCheck(vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()), "Allocating descriptor sets has failed!");
	/// @@ End of creating Descriptor Sets
}

DescriptorSet::~DescriptorSet()
{
	vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
}

void DescriptorSet::Write(size_t descriptorIndex, uint32_t dstBinding, const VkBuffer& buffer, VkDeviceSize range)
{
	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = buffer;
	bufferInfo.offset = 0;
	bufferInfo.range = range;
	
	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSets[descriptorIndex];
	descriptorWrite.dstBinding = dstBinding;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = &bufferInfo;

	vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
}

void DescriptorSet::Write(size_t descriptorIndex, uint32_t dstBinding, const VkImageView& imageView, const VkSampler& sampler)
{
	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = imageView;
	imageInfo.sampler = sampler;

	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSets[descriptorIndex];
	descriptorWrite.dstBinding = dstBinding;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &imageInfo;


	vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
}

void DescriptorSet::CreateDescriptorPoolSize(std::vector<VkDescriptorPoolSize>& poolSizes)
{
	poolSizes.resize(bindingTable.size());

	for (auto bindingIter = bindingTable.cbegin(); bindingIter != bindingTable.cend(); ++bindingIter)
	{
		bool duplicatedType = false;

		for (auto poolSizeIter = poolSizes.begin(); poolSizeIter != poolSizes.end(); ++poolSizeIter)
		{
			// If poolSizes already has the same type of the bindingTable element,
				// Update descriptor count and stop inner loop.
			if (poolSizeIter->type == bindingIter->descriptorType)
			{
				duplicatedType = true;

				poolSizeIter->descriptorCount += bindingIter->descriptorCount * descriptorSetSize;
				break;
			}
		}

		// If there was no element has the same type, 
			// add new element to the pool size.
		if (duplicatedType == false)
		{
			VkDescriptorPoolSize descriptorPoolSize{};
			descriptorPoolSize.type = bindingIter->descriptorType;
			descriptorPoolSize.descriptorCount = bindingIter->descriptorCount * descriptorSetSize;

			poolSizes.push_back(descriptorPoolSize);
		}
	}
}
