/******************************************************************************
Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   Texture.cpp
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 12.19.2022
	Source file for Texture.
******************************************************************************/
#include "Texture.h"
#include <Graphics/Graphics.h>
Texture::Texture(Graphics* graphics, std::string textureName, std::string texturePath)
	: Object(textureName), graphics(graphics)
{
	graphics->CreateTextureImageAndImageView(texturePath, textureImage, textureImageMemory, textureImageView);
}

Texture::~Texture()
{
	Clean();
}

bool Texture::Init()
{
	return false;
}

void Texture::Update(float dt)
{
}

void Texture::Clean()
{
	graphics->DestroyTextureImageAndImageView(textureImage, textureImageMemory, textureImageView);
}

VkImageView Texture::GetImageView()
{
	return textureImageView;
}

void Texture::ChangeTexture(std::string newPath)
{
	graphics->DestroyTextureImageAndImageView(textureImage, textureImageMemory, textureImageView);
	graphics->CreateTextureImageAndImageView(newPath, textureImage, textureImageMemory, textureImageView);
}
