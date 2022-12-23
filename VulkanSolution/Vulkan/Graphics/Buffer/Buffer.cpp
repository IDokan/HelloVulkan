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
#include "Buffer.h"
#include <Graphics/Graphics.h>

Buffer::Buffer(Graphics* graphics, std::string bufferName, VkBufferUsageFlags usage, VkDeviceSize bufferSize, void* data)
	: Object(bufferName), graphics(graphics)
{
	// Use two buffers.
	// One for writing vertex data, the other is actual vertex buffer which we cannot see and use(map) at CPU.
	// The reason why use two buffers is the buffer we can see at CPU is not a good buffer from the GPU side.

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	graphics->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	const VkDevice device = graphics->GetDevice();

	void* data;
	// Operate as glMapBuffer, glUnmapBuffer
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, data, static_cast<size_t>(bufferSize));
	vkUnmapMemory(device, stagingBufferMemory);

	graphics->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMemory);

	graphics->CopyBuffer(stagingBuffer, buffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

Buffer::~Buffer()
{
}

bool Buffer::Init()
{
	return true;
}

void Buffer::Update(float dt)
{
}

void Buffer::Clean()
{
	const VkDevice device = graphics->GetDevice();
	vkDestroyBuffer(device, buffer, nullptr);
	vkFreeMemory(device, bufferMemory, nullptr);
}

const VkBuffer Buffer::GetBuffer()
{
	return buffer;
}

const VkDeviceMemory Buffer::GetBufferMemory()
{
	return bufferMemory;
}
