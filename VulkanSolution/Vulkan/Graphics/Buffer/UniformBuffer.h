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
	UniformBuffer(Graphics* graphics, std::string bufferName, VkDeviceSize bufferSize, int numOfBuffer = 1);
	~UniformBuffer();

	bool Init();
	void Update(float dt);
	void Clean();

	void UpdateUniformData(VkDeviceSize bufferSize, void* data, int i = 0);

	const VkBuffer GetBuffer(int i = 0);
	const VkDeviceMemory GetBufferMemory(int i = 0);
	VkDeviceSize GetBufferSize();
private:
	Graphics* graphics;

	std::vector<VkBuffer> buffers;
	std::vector<VkDeviceMemory> bufferMemories;
	VkDeviceSize bufferSize;
};