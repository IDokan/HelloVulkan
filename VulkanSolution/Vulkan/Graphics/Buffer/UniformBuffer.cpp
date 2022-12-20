/******************************************************************************
Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   Buffer.cpp
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 12.20.2022
	Source file for Buffers.
******************************************************************************/
#include "UniformBuffer.h"
#include <Graphics/Graphics.h>

UniformBuffer::UniformBuffer(Graphics* graphics, std::string bufferName, VkDeviceSize bufferSize)
	: Object(bufferName), graphics(graphics)
{
	// Use two buffers.
	// One for writing vertex data, the other is actual vertex buffer which we cannot see and use(map) at CPU.
	// The reason why use two buffers is the buffer we can see at CPU is not a good buffer from the GPU side.

	graphics->CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		buffer, bufferMemory);
}

UniformBuffer::~UniformBuffer()
{
}

bool UniformBuffer::Init()
{
	return true;
}

void UniformBuffer::Update()
{
}

void UniformBuffer::Clean()
{
	const VkDevice device = graphics->GetDevice();
	vkDestroyBuffer(device, buffer, nullptr);
	vkFreeMemory(device, bufferMemory, nullptr);
}

void UniformBuffer::UpdateUniformData(VkDeviceSize bufferSize, void* data)
{
	const VkDevice device = graphics->GetDevice();

	void* data;
	vkMapMemory(device, bufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, &data, bufferSize);
	vkUnmapMemory(device, bufferMemory);
}

const VkBuffer UniformBuffer::GetBuffer()
{
	return buffer;
}

const VkDeviceMemory UniformBuffer::GetBufferMemory()
{
	return bufferMemory;
}
