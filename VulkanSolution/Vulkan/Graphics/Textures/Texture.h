/******************************************************************************
Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   Texture.h
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 12.19.2022
	Header file for Texture.
******************************************************************************/

#pragma once
#include <Engines/Objects/Object.h>
#include <vulkan/vulkan.h>
#include <string>

class Graphics;

class Texture : public Object
{
public:
	Texture(Graphics* graphics, std::string textureName, std::string texturePath);
	~Texture();

	bool Init();
	void Update(float dt);
	void Clean();

	VkImageView GetImageView();

	void ChangeTexture(std::string newPath);

private:
	Graphics* graphics;
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
};