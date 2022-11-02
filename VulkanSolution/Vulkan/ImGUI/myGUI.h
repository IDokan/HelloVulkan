/******************************************************************************
Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   myGUI.h
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 10.06.2022
	header file for dear ImGUI customization.
******************************************************************************/
#pragma once
#include "vulkan/vulkan.h"

struct GLFWwindow;
class Model;

namespace MyImGUI
{
    // Should be called after initialization of Vulkan
	void InitImGUI(GLFWwindow* window, VkDevice device, VkInstance instance, 
		VkPhysicalDevice physicalDevice, VkQueue queue, VkRenderPass renderPass, 
		VkCommandBuffer commandBuffer);

    void SendModelInfo(Model* model);
    void SendSkeletonInfo(bool* showSkeletonFlag);
    void SendAnimationInfo(float* worldTimer, bool* bindPoseFlag);

    void UpdateAnimationNameList();

    // Should be called before destruction of Vulkan
    void DestroyGUIResources();

    // Where implement ImGUI functions calls
    void DrawGUI();

    // Function should be called every frame
    void GUIRender(VkCommandBuffer commandBuffer);
}