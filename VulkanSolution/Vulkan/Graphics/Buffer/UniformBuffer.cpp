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
#include <vector>
#include "UniformBuffer.h"
#include <Graphics/Graphics.h>

UniformBuffer::UniformBuffer(Graphics* graphics, std::string bufferName, VkDeviceSize bufferSize, int numOfBuffer)
	: Object(bufferName), graphics(graphics), bufferSize(bufferSize)
{
	// Use two buffers.
	// One for writing vertex data, the other is actual vertex buffer which we cannot see and use(map) at CPU.
	// The reason why use two buffers is the buffer we can see at CPU is not a good buffer from the GPU side.

	buffers.resize(numOfBuffer);
	bufferMemories.resize(numOfBuffer);

	for (int i = 0; i < numOfBuffer; i++)
	{
		graphics->CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			buffers[i], bufferMemories[i]);
	}
}

UniformBuffer::~UniformBuffer()
{
}

bool UniformBuffer::Init()
{
	return true;
}

void UniformBuffer::Update(float dt)
{
}

void UniformBuffer::Clean()
{
	const VkDevice device = graphics->GetDevice();

	for (VkBuffer& buffer : buffers)
	{
		vkDestroyBuffer(device, buffer, nullptr);
	}
	for (VkDeviceMemory& bufferMemory : bufferMemories)
	{
		vkFreeMemory(device, bufferMemory, nullptr);
	}
}

void UniformBuffer::UpdateUniformData(VkDeviceSize bufferSize, void* data, int i)
{
	const VkDevice device = graphics->GetDevice();

	void* memory;
	vkMapMemory(device, bufferMemories[i], 0, bufferSize, 0, &memory);
	memcpy(memory, data, bufferSize);
	vkUnmapMemory(device, bufferMemories[i]);
}

const VkBuffer UniformBuffer::GetBuffer(int i)
{
	return buffers[i];
}

const VkDeviceMemory UniformBuffer::GetBufferMemory(int i)
{
	return bufferMemories[i];
}

VkDeviceSize UniformBuffer::GetBufferSize()
{
	return bufferSize;
}
