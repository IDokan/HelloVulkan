/******************************************************************************
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   VulkanHelper.h
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 06.12.2021
	header file for vulkan helper.
		- result checks
******************************************************************************/
#pragma once
#include <iostream>

enum VkResult;

namespace VulkanHelper
{
	VkResult VkCheck(VkResult result, const char* errorMsg);


	inline VkResult VulkanHelper::VkCheck(VkResult result, const char* errorMsg)
	{
#ifdef _DEBUG
		if (result != VK_SUCCESS)
		{
			std::cout << errorMsg << std::endl;
			abort();
		}
#endif
		return result;
	}

// #define ARRAYSIZE(ARR) sizeof(ARR) / sizeof(ARR ## [0])
}