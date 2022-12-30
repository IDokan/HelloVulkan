/******************************************************************************
Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   Buffer.h
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 12.20.2022
	Header file for Buffers.
******************************************************************************/
#pragma once
#include <Engines/Objects/Object.h>
#include <vulkan/vulkan.h>

class Graphics;

class Buffer : public Object
{
public:
	Buffer(Graphics* graphics, std::string bufferName, VkBufferUsageFlags usage, unsigned int dataTypeSize, size_t dataSize, void* data);
	~Buffer();

	bool Init();
	void Update(float dt);
	void Clean();

	unsigned int GetBufferDataTypeSize();
	size_t GetBufferDataSize();
	VkBuffer GetBuffer();
	VkDeviceMemory GetBufferMemory();

	void ChangeBufferData(unsigned int dataTypeSize, size_t dataSize, void* data);
private:
	Graphics* graphics;

	VkBufferUsageFlags usage;

	unsigned int dataTypeSize;
	size_t dataSize;

	VkBuffer buffer;
	VkDeviceMemory bufferMemory;
};