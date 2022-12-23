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

#include <Engines/Objects/Object.h>
#include <vulkan/vulkan.h>

class Graphics;

class Buffer : public Object
{
public:
	Buffer(Graphics* graphics, std::string bufferName, VkBufferUsageFlags usage, VkDeviceSize bufferSize, void* data);
	~Buffer();

	bool Init();
	void Update(float dt);
	void Clean();

	const VkBuffer GetBuffer();
	const VkDeviceMemory GetBufferMemory();
private:
	Graphics* graphics;

	VkBuffer buffer;
	VkDeviceMemory bufferMemory;
};