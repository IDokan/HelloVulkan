/******************************************************************************
Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   UniformBuffer.h
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 12.20.2022
	Header file for Buffers.
******************************************************************************/

#include <Engines/Objects/Object.h>
#include <vulkan/vulkan.h>

class Graphics;

class UniformBuffer : public Object
{
public:
	UniformBuffer(Graphics* graphics, std::string bufferName, VkDeviceSize bufferSize);
	~UniformBuffer();

	bool Init();
	void Update();
	void Clean();

	void UpdateUniformData(VkDeviceSize bufferSize, void* data);

	const VkBuffer GetBuffer();
	const VkDeviceMemory GetBufferMemory();
private:
	Graphics* graphics;

	VkBuffer buffer;
	VkDeviceMemory bufferMemory;
};