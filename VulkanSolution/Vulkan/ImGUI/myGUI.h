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
#include "glm/vec3.hpp"

struct Vertex;
struct GLFWwindow;
class Model;
class HairBone;

namespace MyImGUI
{
    // Should be called after initialization of Vulkan
	void InitImGUI(GLFWwindow* window, VkDevice device, VkInstance instance, 
		VkPhysicalDevice physicalDevice, VkQueue queue, VkRenderPass renderPass, 
		VkCommandBuffer commandBuffer);

    void SendModelInfo(Model* model, bool* showModel, bool* vertexPointsMode, float* pointSize, int* selectedMesh);
    void SendSkeletonInfo(bool* showSkeletonFlag, bool* blendingWeightMode, int* selectedBone);
    void SendAnimationInfo(float* worldTimer, bool* bindPoseFlag, bool* playAnimation);
    void SendConfigInfo(float* mouseSensitivity);
    void SendHairBoneInfo(HairBone* hairBone, char* newBoneName, size_t boneContainerNameSize, bool* applyingBone, float* sphereTrans, float min, float max, float* sphereRadius, int* boneIDIndex, float* boneWeight, bool* flagChange);

    void UpdateClickedVertexAddress(Vertex* vertex);
    void UpdateAnimationNameList();
    void UpdateBoneNameList();
    void UpdateMeshNameList();
    void UpdateSphereMinMaxRange(float min, float max);
    

    bool IsMouseOnImGUIWindow();

    // Should be called before destruction of Vulkan
    void DestroyGUIResources();

    // Where implement ImGUI functions calls
    void DrawGUI();

    // Function should be called every frame
    void GUIRender(VkCommandBuffer commandBuffer);
}
