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
#include "ImGUI/myGUI.h"
#include "Helper/VulkanHelper.h"
#include "ImGUI/backends/imgui_impl_glfw.h"
#include "ImGUI/imgui.h"
#include "ImGUI/backends/imgui_impl_vulkan.h"
#include <xutility>
#include "Engines/Window.h"
#include "Graphics/Model/Model.h"

VkDescriptorPool imguiDescriptorPool{ VK_NULL_HANDLE };
VkDevice guiDevice;

namespace MyImGUI
{
    namespace Helper
    {
        void ModelStats();
        void BlendingWeightsSkeletonSelectionRecursively(int currentBoneIndex, int boneSize, ImGuiTreeNodeFlags baseFlags, bool nodeOpened = false);
        void Skeleton();
        void Animation();
    }
}

namespace
{
    Model* model;
    float* worldTimer;
    std::vector<std::string> animationNameList;
    
    std::vector<std::string> boneNameList;
    // pair<bone ID, parent ID>
    std::vector<std::pair<int, int>> boneIdPid;
    bool* bindPoseFlag;

    bool* showSkeletonFlag;
    unsigned int selected = 0;
    bool* blendingWeightMode;
    int* selectedBone;
}

namespace MyImGUI
{
    void SendIMGUIFontTextureToGPU(VkDevice device, VkCommandBuffer commandBuffer, VkQueue queue);
}
void MyImGUI::InitImGUI(GLFWwindow* window, VkDevice device, VkInstance instance, VkPhysicalDevice physicalDevice, VkQueue queue, VkRenderPass renderPass, VkCommandBuffer commandBuffer)
{
    guiDevice = device;

    // 1. create descriptor pool for IMGUI
        // the size of the pool is very oversize, but it's copied from imgui demo itself
    VkDescriptorPoolSize poolSizes[] =
    {
        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
    };



    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000;
    poolInfo.poolSizeCount = static_cast<uint32_t>(std::size(poolSizes));
    poolInfo.pPoolSizes = poolSizes;

    VulkanHelper::VkCheck(vkCreateDescriptorPool(device, &poolInfo, nullptr, &imguiDescriptorPool), "Creating descriptor pool has failed!");

    // 2. Initialize imgui library

    // it initializes the core structures of imgui
    ImGui::CreateContext();
    // it initializes imgui for GLFW
    ImGui_ImplGlfw_InitForVulkan(window, true);

    // it initializes imgui for Vulkan
    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance = instance;
    initInfo.PhysicalDevice = physicalDevice;
    initInfo.Device = device;
    initInfo.Queue = queue;
    initInfo.DescriptorPool = imguiDescriptorPool;
    initInfo.MinImageCount = 3;
    initInfo.ImageCount = 3;
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&initInfo, renderPass);

    // TODO: send imgui font textures to GPU
    SendIMGUIFontTextureToGPU(device, commandBuffer, queue);

    // Clear font textures from cpu data

    // add the destroy the imgui created structures
}

void MyImGUI::SendIMGUIFontTextureToGPU(VkDevice device, VkCommandBuffer commandBuffer, VkQueue queue)
{
    // execute a GPU command to upload imgui font textures
    VkCommandBufferBeginInfo cmdBeginInfo{};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.pNext = nullptr;
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    cmdBeginInfo.pInheritanceInfo = nullptr;

    VulkanHelper::VkCheck(vkBeginCommandBuffer(commandBuffer, &cmdBeginInfo), "Beginning command buffer has failed!");

    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

    VulkanHelper::VkCheck(vkEndCommandBuffer(commandBuffer), "Ending command buffer has failed!");

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;


    VkFenceCreateInfo fenceCreateInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    fenceCreateInfo.pNext = nullptr;


    VkFence submitFence;
    vkCreateFence(device, &fenceCreateInfo, nullptr, &submitFence);

    VulkanHelper::VkCheck(vkQueueSubmit(queue, 1, &submitInfo, submitFence), "Submitting queue has failed!");

    vkWaitForFences(device, 1, &submitFence, true, 9999999999);
    vkResetFences(device, 1, &submitFence);

    vkDestroyFence(device, submitFence, nullptr);
}

void MyImGUI::DestroyGUIResources()
{

    VulkanHelper::VkCheck(vkDeviceWaitIdle(guiDevice), "failed to make logical device idle");

    vkDestroyDescriptorPool(guiDevice, imguiDescriptorPool, nullptr);
    ImGui_ImplVulkan_Shutdown();
}

void MyImGUI::DrawGUI()
{
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::ShowDemoWindow();
    

    ImGui::Begin("Controller");

    Helper::ModelStats();
    Helper::Skeleton();
    Helper::Animation();

    ImGui::End();
    ImGui::EndFrame();
}

void MyImGUI::GUIRender(VkCommandBuffer commandBuffer)
{
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

void MyImGUI::Helper::ModelStats()
{
    if (ImGui::CollapsingHeader("Basic Information"))
    {
        const int meshSize = model->GetMeshSize();
        for (int i = 0; i < meshSize; i++)
        {
            if (ImGui::TreeNode(model->GetMeshName(i).c_str()))
            {
                ImGui::TextWrapped("Vertex Count: %d", model->GetVertexCount(i));
                ImGui::TextWrapped("Triangle Count: %d", model->GetIndexCount(i) / 3);

                ImGui::TreePop();
            }
        }
    }
}

void MyImGUI::Helper::BlendingWeightsSkeletonSelectionRecursively(int currentBoneIndex, int boneSize, ImGuiTreeNodeFlags baseFlags, bool nodeOpened)
{
    // Safe check
    if (currentBoneIndex < 0 || currentBoneIndex >= boneSize)
    {
        return;
    }

    // Flag controls
    ImGuiTreeNodeFlags nodeFlags = baseFlags;
    if (*selectedBone == currentBoneIndex)
    {
        nodeFlags |= ImGuiTreeNodeFlags_Selected;
    }
    if (nodeOpened)
    {
        nodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
    }

    bool nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)currentBoneIndex, nodeFlags, boneNameList[currentBoneIndex].c_str());
    if (ImGui::IsItemClicked())
    {
        *selectedBone = currentBoneIndex;
    }

    // Call children bones recursively if node selected
    if (nodeOpen)
    {
        // Use temporary vector to automatically open a single child node.
        std::vector<int> childBoneIndices;
        for (int i = 0; i < boneSize; i++)
        {
            if (boneIdPid[i].second == currentBoneIndex)
            {
                childBoneIndices.push_back(boneIdPid[i].first);
            }
        }

        size_t childCount = childBoneIndices.size();
        // Automatically open when it has only one child.
        if (childCount == 1)
        {
            BlendingWeightsSkeletonSelectionRecursively(childBoneIndices[0], boneSize, baseFlags, true);
        }
        // Call default recursive calls when it has children.
        else
        {
            for (size_t i = 0; i < childBoneIndices.size(); i++)
            {
                BlendingWeightsSkeletonSelectionRecursively(childBoneIndices[i], boneSize, baseFlags);
            }
        }
        ImGui::TreePop();
    }
}

void MyImGUI::Helper::Skeleton()
{
    if (ImGui::CollapsingHeader("Skeleton"))
    {
        ImGui::Checkbox("Show skeleton", showSkeletonFlag);

        ImGui::Separator();
        ImGui::Checkbox("Blending Weight Mode", blendingWeightMode);
        if (*blendingWeightMode)
        {

            ImGui::Indent();
            const unsigned int boneCount = static_cast<unsigned int>(model->GetBoneCount());
            static ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_None | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
            BlendingWeightsSkeletonSelectionRecursively(0, boneCount, baseFlags);
            ImGui::Unindent();
        }
    }
}

void MyImGUI::SendModelInfo(Model* _model)
{
    model = _model;
}

void MyImGUI::SendSkeletonInfo(bool* _showSkeletonFlag, bool* _blendingWeightMode, int* _selectedBone)
{
    showSkeletonFlag = _showSkeletonFlag;
    blendingWeightMode = _blendingWeightMode;
    selectedBone = _selectedBone;
}

void MyImGUI::SendAnimationInfo(float* _worldTimer, bool* _bindPoseFlag)
{
    worldTimer = _worldTimer;
    bindPoseFlag = _bindPoseFlag;
}

void MyImGUI::UpdateAnimationNameList()
{
    unsigned int previousSelected = model->GetSelectedAnimationIndex();

    const unsigned int animationCount = model->GetAnimationCount();

    // 1 is for bind pose
    animationNameList.resize(animationCount + 1);

    // Extract animation names
    for (unsigned int i = 0; i < animationCount; ++i)
    {
        model->SetAnimationIndex(i);
        animationNameList[i] = model->GetAnimationName();
    }

    animationNameList[animationCount] = "Bind Pose";

    // Recover selected animation index
    model->SetAnimationIndex(previousSelected);

    // Default selection is always bind pose
    selected = animationCount;
    *bindPoseFlag = true;
}

void MyImGUI::UpdateBoneNameList()
{
    const unsigned int boneCount = static_cast<unsigned int>(model->GetBoneCount());
    boneNameList.clear();
    boneNameList.resize(boneCount);
    boneIdPid.clear();
    boneIdPid.resize(boneCount);

    for (unsigned int i = 0; i < boneCount; i++)
    {
        const Bone& bone = model->GetBone(i);
        boneNameList[i] = bone.name;
        boneIdPid[i].first = bone.id;
        boneIdPid[i].second = bone.parentID;
    }
}

bool MyImGUI::IsMouseOnImGUIWindow()
{
    return ImGui::GetIO().WantCaptureMouse;
}


void MyImGUI::Helper::Animation()
{
    if (ImGui::CollapsingHeader("Animation"))
    {
        ImGui::TextWrapped("Select animation by Name");
        ImGui::Indent();
        const unsigned int animationCount = model->GetAnimationCount();
        for (unsigned int n = 0; n <= animationCount; n++)
        {
            if (ImGui::Selectable(animationNameList[n].c_str(), selected == n))
            {
                selected = n;
                if (selected == animationCount)
                {
                    *bindPoseFlag = true;
                }
                else
                {
                    model->SetAnimationIndex(selected);
                    *bindPoseFlag = false;
                }
            }
        }
        ImGui::Unindent();

        if (*bindPoseFlag == false)
        {
            ImGui::Separator();
            ImGui::TextWrapped("Duration:");
            float duration = model->GetAnimationDuration();
            ImGui::TextWrapped(("\t" + std::to_string(duration)).c_str());
            ImGui::SliderFloat("Animation Time", worldTimer, 0.f, duration);
        }
    }
}
