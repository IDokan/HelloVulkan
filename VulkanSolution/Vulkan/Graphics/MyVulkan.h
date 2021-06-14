/******************************************************************************
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   MyVulkan.h
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 06.12.2021
	header file for my vulkan.
******************************************************************************/
#pragma once

namespace MyVulkan
{
	// Return if initialization is succeed or not
	// Use VK_MAKE_VERSION(major, minor, patch) for second parameter 'appVersion'
	void InitVulkan(const char* appName, uint32_t appVersion);
	void CleanVulkan();

	void CreateBuffers();
	void CreateImages();
}