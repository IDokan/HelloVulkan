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
#include "ImGUI/imgui_internal.h"
#include "Helper/VulkanHelper.h"
#include "ImGUI/backends/imgui_impl_glfw.h"
#include "ImGUI/imgui.h"
#include "ImGUI/backends/imgui_impl_vulkan.h"
#include <xutility>
#include "Engines/Window.h"
#include "Graphics/Model/Model.h"
#include <Graphics/Structures/Structs.h>
#include <Engines/Objects/HairBone.h>

VkDescriptorPool imguiDescriptorPool{ VK_NULL_HANDLE };
VkDevice guiDevice;

namespace MyImGUI
{
    namespace Helper
    {
        void ModelStats();
        void BoneEditor();
        void VertexSpectator();
        void HairBoneInspector();
        void VertexSelectionSphereManipulator();
        void BlendingWeightsSkeletonSelectionRecursively(int currentBoneIndex, int boneSize, ImGuiTreeNodeFlags baseFlags, bool nodeOpened = false);
        void Skeleton();
        void Animation();
        void Physics();
        void Configuration();
    }
}

namespace
{
    Model* model;
    bool* showModel;
    bool* vertexPointsMode;
    float* pointSize;
    float* worldTimer;
    std::vector<std::string> animationNameList;

    std::vector<std::string> boneNameList;
    // pair<bone ID, parent ID>
    std::vector<std::pair<int, int>> boneIdPid;
    bool* bindPoseFlag;
    bool* playAnimation;

    bool* showSkeletonFlag;
    unsigned int selected = 0;
    bool* blendingWeightMode;
    int* selectedBone;

    std::vector<std::string> meshNameList;
    int* selectedMesh;

    float* mouseSensitivity;

    // Don't use it to set clicked vertex's data
    Vertex* clickedVertex = nullptr;

    HairBone* hairBone = nullptr;
    int selectedHairBone = 0;
    char* newBoneName = nullptr;
    size_t boneNameContainerSize = 0;
    bool* applyingBoneRef = nullptr;
    float* sphereTrans = nullptr;
    float minimum;
    float maximum;
    float* sphereRadius = nullptr;
    int* boneIDIndex = nullptr;
    float* boneWeights = nullptr;
    bool* flagChangeBoneIndexInSphere = nullptr;

    bool* runRealtime;
    bool* proceedFrame;

    bool* cleanBones;
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

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

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



    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
    ImGui::Begin("DockSpace", nullptr, windowFlags);
    ImGui::PopStyleVar();
    ImGui::PopStyleVar(2);

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspaceID = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspaceID, ImVec2(0.f, 0.f), ImGuiDockNodeFlags_PassthruCentralNode);

        static auto firstTime = true;
        if (firstTime)
        {
            firstTime = false;

            ImGui::DockBuilderRemoveNode(dockspaceID);
            ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspaceID, viewport->Size);

            auto dockIDLeft = ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Left, 0.32f, nullptr, &dockspaceID);
            ImGui::DockBuilderDockWindow("Controller", dockIDLeft);
            ImGui::DockBuilderFinish(dockspaceID);
        }
    }
    ImGui::End();

    // ImGui::ShowDemoWindow();


    ImGui::Begin("Controller");

    Helper::ModelStats();
    Helper::Skeleton();
    Helper::BoneEditor();
    Helper::Animation();
    Helper::Physics();
    Helper::Configuration();

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
        ImGui::Checkbox("Display model", showModel);
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

        ImGui::Separator();
        ImGui::Checkbox("Display Vertices", vertexPointsMode);
        if (*vertexPointsMode)
        {
            ImGui::SliderFloat("Vertex Size", pointSize, 1.f, 10.f);

            std::string preview = meshNameList[*selectedMesh];
            if (ImGui::BeginCombo("Display mesh", preview.c_str()))
            {
                for (int i = 0; i < meshNameList.size(); i++)
                {
                    const bool isSelected = (*selectedMesh == i);
                    if (ImGui::Selectable(meshNameList[i].c_str(), isSelected))
                    {
                        *selectedMesh = i;
                    }
                    if (isSelected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::Separator();

            Helper::VertexSpectator();
        }
        else
        {
            // Initialize selected mesh parameter, if display vertex is turned off.
            *selectedMesh = meshNameList.size() - 1;
        }
    }
}

void MyImGUI::Helper::BoneEditor()
{
    if (ImGui::CollapsingHeader("Bone Editor"))
    {
        bool isMeshNotSelected = (*selectedMesh == meshNameList.size() - 1);
        bool isBoneNotSelected = (*blendingWeightMode == false);
        if (isMeshNotSelected || isBoneNotSelected)
        {
            if (isMeshNotSelected)
            {
                ImGui::Text("Please select a mesh in \n\tBasicInformation::DisplayVertices");
            }
            if (isBoneNotSelected)
            {
                ImGui::Text("Please select a bone in \n\tSkeleton::BlendingWeightMode");
            }
            return;
        }

        Helper::HairBoneInspector();
        ImGui::Separator();
        Helper::VertexSelectionSphereManipulator();

        ImGui::Separator();

        if (ImGui::Button("Clean bones"))
        {
            *cleanBones = true;
        }

    }
}

void MyImGUI::Helper::VertexSpectator()
{
    // Do not output vertex data if selected mesh == meshNameList size - 1
    if (*selectedMesh == meshNameList.size() - 1)
    {
        clickedVertex == nullptr;
    }
    if (clickedVertex == nullptr)
    {
        ImGui::Text("Please click a vertex to inspect");
        return;
    }

    ImGui::BulletText("Position) %f, %f, %f", clickedVertex->position.x, clickedVertex->position.y, clickedVertex->position.z);
    ImGui::BulletText("Normal) %f, %f, %f", clickedVertex->normal.x, clickedVertex->normal.y, clickedVertex->normal.z);
    ImGui::BulletText("Vertex Color) %f, %f, %f", clickedVertex->vertexColor.x, clickedVertex->vertexColor.y, clickedVertex->vertexColor.z);
    ImGui::BulletText("Tex Coord) %f, %f", clickedVertex->texCoord.x, clickedVertex->texCoord.y);
    ImGui::BulletText("Bone ID) %d, %d, %d, %d", clickedVertex->boneIDs.x, clickedVertex->boneIDs.y, clickedVertex->boneIDs.z, clickedVertex->boneIDs.w);
    ImGui::BulletText("Bone Weights) %f, %f, %f, %f", clickedVertex->boneWeights.x, clickedVertex->boneWeights.y, clickedVertex->boneWeights.z, clickedVertex->boneWeights.w);
}

void MyImGUI::Helper::HairBoneInspector()
{
    ImGui::InputText("Bone Name", newBoneName, boneNameContainerSize);

    glm::vec4* data = reinterpret_cast<glm::vec4*>(hairBone->GetBoneData());
    glm::vec3 min;
    glm::vec3 max;
    model->GetBoundingBoxMinMax(min, max);

    ImGui::SliderFloat("Bone Position X", &data[selectedHairBone].x, -max.x, max.x);
    ImGui::SliderFloat("Bone Position Y", &data[selectedHairBone].y, -max.y, max.y);
    ImGui::SliderFloat("Bone Position Z", &data[selectedHairBone].z, -max.z, max.z);

    if (ImGui::Button("Apply Bone"))
    {
        *applyingBoneRef = true;
    }
}

void MyImGUI::Helper::VertexSelectionSphereManipulator()
{

    ImGui::InputInt("Bone ID index", boneIDIndex);
    std::clamp(*boneIDIndex, 0, 3);

    glm::vec3 scale = model->GetModelScale();

    ImGui::SliderFloat("Sphere Radius", sphereRadius, 0.f, std::max(std::max(scale.x, scale.y), scale.z));
    ImGui::InputFloat3("Sphere Position", sphereTrans);

    ImGui::SliderFloat4("Bone Weights", boneWeights, 0.f, 1.f);
    if (ImGui::Button("Apply selected bone"))
    {
        // @@ TODO: Add applying bone weight in the sphere
        //clickedVertex->boneIDs[*boneIDIndex] = *selectedBone;
        *flagChangeBoneIndexInSphere = true;
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
            ImGui::Separator();
            ImGui::Text("Select a bone to visualize weights");
            ImGui::Indent();
            const unsigned int boneCount = static_cast<unsigned int>(model->GetBoneCount());
            static ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_None | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
            BlendingWeightsSkeletonSelectionRecursively(0, boneCount, baseFlags);
            ImGui::Unindent();

            Bone* selectedBonePtr = const_cast<Bone*>(model->GetBone(*selectedBone));
            if (JiggleBone* jiggleBonePtr = dynamic_cast<JiggleBone*>(selectedBonePtr);
                jiggleBonePtr != nullptr)
            {
                static bool isUpdatePrevious = false;
                static bool isUpdateAfter = false;
                isUpdatePrevious = jiggleBonePtr->isUpdateJigglePhysics;
                isUpdateAfter = isUpdatePrevious;
                ImGui::Checkbox("Set physics", &isUpdateAfter);
                if (isUpdatePrevious != isUpdateAfter)
                {
                    jiggleBonePtr->SetIsUpdateJigglePhysics(isUpdateAfter);
                }
            }
        }
    }
}

void MyImGUI::SendModelInfo(Model* _model, bool* _showModel, bool* _vertexPointsMode, float* _pointSize, int* _selectedMesh)
{
    model = _model;
    showModel = _showModel;
    vertexPointsMode = _vertexPointsMode;
    pointSize = _pointSize;
    selectedMesh = _selectedMesh;
}

void MyImGUI::SendSkeletonInfo(bool* _showSkeletonFlag, bool* _blendingWeightMode, int* _selectedBone, bool* cleanBoneFlag)
{
    showSkeletonFlag = _showSkeletonFlag;
    blendingWeightMode = _blendingWeightMode;
    selectedBone = _selectedBone;
    cleanBones = cleanBoneFlag;
}

void MyImGUI::SendAnimationInfo(float* _worldTimer, bool* _bindPoseFlag, bool* _playAnimation)
{
    worldTimer = _worldTimer;
    bindPoseFlag = _bindPoseFlag;
    playAnimation = _playAnimation;
}

void MyImGUI::SendConfigInfo(float* _mouseSensitivity)
{
    mouseSensitivity = _mouseSensitivity;
}

void MyImGUI::SendHairBoneInfo(HairBone* _hairBone, char* _newBoneName, size_t _boneContainerNameSize, bool* applyingBone, float* _sphereTrans, float min, float max, float* _sphereRadius, int* _boneIDIndex, float* _boneWeight, bool* _flagChange)
{
    hairBone = _hairBone;
    newBoneName = _newBoneName;
    boneNameContainerSize = _boneContainerNameSize;
    applyingBoneRef = applyingBone;
    sphereTrans = _sphereTrans;
    UpdateSphereMinMaxRange(min, max);
    sphereRadius = _sphereRadius;
    boneIDIndex = _boneIDIndex;
    boneWeights = _boneWeight;
    flagChangeBoneIndexInSphere = _flagChange;
}

void MyImGUI::SendPhysicsInfo(bool* _runRealtime, bool* _proceedFrame)
{
    runRealtime = _runRealtime;
    proceedFrame = _proceedFrame;
}

void MyImGUI::UpdateClickedVertexAddress(Vertex* _vertex)
{
    clickedVertex = _vertex;
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
        const Bone* bone = model->GetBone(i);
        boneNameList[i] = bone->name;
        boneIdPid[i].first = bone->id;
        boneIdPid[i].second = bone->parentID;
    }
}

void MyImGUI::UpdateMeshNameList()
{
    const int meshSize = model->GetMeshSize();
    meshNameList.resize(meshSize + 1);

    for (int i = 0; i < meshSize; i++)
    {
        meshNameList[i] = model->GetMeshName(i);
    }
    meshNameList[meshSize] = "All meshes";
    *selectedMesh = meshSize;
}

void MyImGUI::UpdateSphereMinMaxRange(float min, float max)
{
    minimum = min;
    maximum = max;
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
            ImGui::Checkbox("play animation?", playAnimation);
        }
    }
}

void MyImGUI::Helper::Physics()
{
    if (ImGui::CollapsingHeader("Physics"))
    {
        ImGui::SliderFloat("Gravity Scaler", &Physics::GravityScaler, 1.f, 10.f);
        ImGui::SliderFloat3("Gravity Vector", &Physics::GravityVector.x, -1.f, 1.f);

        
        if (const JiggleBone* jb = dynamic_cast<const JiggleBone*>(model->GetBone(*selectedBone));
            jb != nullptr)
        {
            JiggleBone* jiggleBone = const_cast<JiggleBone*>(jb);
            ImGui::SliderFloat("Spring Scaler", &jiggleBone->physics.springScaler, 1.f, 500.f);
            ImGui::SliderFloat("Damping Scaler", &jiggleBone->physics.dampingScaler, 1.f, 500.f);
            ImGui::SliderFloat("Bending Spring Scaler", &jiggleBone->physics.bendSpringScaler, 1.f, 500.f);
            ImGui::SliderFloat("Bending Damping Scaler", &jiggleBone->physics.bendDampingScaler, 1.f, 500.f);
        }

        ImGui::Separator();
        ImGui::Checkbox("Run realtime", runRealtime);
        if (ImGui::Button("Proceed A Frame"))
        {
            *proceedFrame = true;
        }

        ImGui::Checkbox("Apply force flag", &Physics::forceApplyFlag);
    }
}



void MyImGUI::Helper::Configuration()
{
    if (ImGui::CollapsingHeader("Configuration"))
    {
        ImGui::SliderFloat("Mouse Sensitivity", mouseSensitivity, 1.f, 100.f);
    }
}
