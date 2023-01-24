/******************************************************************************
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   MyScene.cpp
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 06.12.2021
	Source file for my vulkan.
******************************************************************************/
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>	// for glfw functions such as glfwGetRequiredInstanceExtensions, glfwCreateWindowSurface, ....
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm> // for std::clamp

#include "Graphics/MyScene.h"
#include "Graphics.h"
#include "Helper/VulkanHelper.h"
#include "GLMath.h"
#include <chrono>
#include "Engines/Window.h"
#include "Graphics/Structures/Structs.h"
#include "Graphics/Model/Model.h"
#include "Graphics/DescriptorSet.h"
#include "ImGUI/myGUI.h"
#include "Engines/Input/Input.h"

#include <Engines/Objects/Object.h>
#include <Graphics/Textures/Texture.h>
#include <Graphics/Buffer/Buffer.h>
#include <Graphics/Buffer/UniformBuffer.h>
#include <Graphics/Pipelines/Pipeline.h>
#include <Engines/Objects/HairBone.h>

MyScene::MyScene(Window* window)
	: windowHolder(window), model(nullptr), timer(0.f), rightMouseCenter(glm::vec3(0.f, 0.f, 0.f)), cameraPoint(glm::vec3(0.f, 0.f, 2.f)), targetPoint(glm::vec3(0.f)), bindPoseFlag(false), showSkeletonFlag(true), blendingWeightMode(false), showModel(true), vertexPointsMode(false), pointSize(5.f), selectedMesh(0), mouseSensitivity(1.f), applyingBone(false)
{
}

bool MyScene::InitScene(Graphics* _graphics)
{
	model = new Model("../Vulkan/Graphics/Model/models/Dancing.fbx");
	model->SetAnimationIndex(0);
	selectedMesh = model->GetMeshSize();

	graphics = _graphics;

	graphicResources.push_back(new Texture(graphics, "EmergencyTexture", "../Vulkan/Graphics/Model/EssentialImages/transparent.png"));
	const std::vector<std::string>& paths = model->GetDiffuseImagePaths();
	for (int i = 0; i < paths.size(); i++)
	{
		graphicResources.push_back(new Texture(graphics, std::string("diffuseImage") + std::to_string(i), paths[i]));
	}
	graphicResources.push_back(new Buffer(graphics, "skeletonBuffer", VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(LineVertex), 2 * model->GetBoneCount(), model->GetBoneDataForDrawing()));
	const int meshSize = model->GetMeshSize();
	for (int i = 0; i < meshSize; i++)
	{
		graphicResources.push_back(new Buffer(graphics, std::string("vertex") + std::to_string(i), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(Vertex), model->GetVertexCount(i), model->GetVertexData(i)));
		graphicResources.push_back(new Buffer(graphics, std::string("index") + std::to_string(i), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, sizeof(uint32_t), model->GetIndexCount(i), model->GetIndexData(i)));
		graphicResources.push_back(new Buffer(graphics, std::string("uniqueVertex") + std::to_string(i), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(Vertex), model->GetUniqueVertexCount(i), model->GetUniqueVertexData(i)));
	}

	graphicResources.push_back(new UniformBuffer(graphics, std::string("uniformBuffer"), sizeof(UniformBufferObject), Graphics::MAX_FRAMES_IN_FLIGHT));
	graphicResources.push_back(new UniformBuffer(graphics, std::string("animationUniformBuffer"), sizeof(glm::mat4) * model->GetBoneCount(), Graphics::MAX_FRAMES_IN_FLIGHT));
	graphicResources.push_back(new UniformBuffer(graphics, std::string("unitBoneUniformBuffer"), sizeof(glm::mat4) * model->GetBoneCount(), Graphics::MAX_FRAMES_IN_FLIGHT));

	graphicResources.push_back(new DescriptorSet(graphics, "descriptor", Graphics::MAX_FRAMES_IN_FLIGHT * meshSize, {
		{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
		{1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
		{2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}
		}));
	WriteDescriptorSet();

	graphicResources.push_back(new DescriptorSet(graphics, "waxDescriptor", Graphics::MAX_FRAMES_IN_FLIGHT * meshSize, {
		{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
		{1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}
		}));

	WriteWaxDescriptorSet();

	graphicResources.push_back(new DescriptorSet(graphics, "blendingWeightDescriptor", Graphics::MAX_FRAMES_IN_FLIGHT * meshSize, {
		{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
		{1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}
		}));
	WriteBlendingWeightDescriptorSet();

	DescriptorSet* descriptorSet = dynamic_cast<DescriptorSet*>(FindObjectByName("descriptor"));
	graphicResources.push_back(new Pipeline(graphics, "pipeline", "spv/vertexShader.vert.spv", "spv/fragShader.frag.spv", descriptorSet->GetDescriptorSetLayoutPtr()));

	DescriptorSet* waxDescriptorSet = dynamic_cast<DescriptorSet*>(FindObjectByName("waxDescriptor"));
	graphicResources.push_back(new Pipeline(graphics, "waxPipeline", "spv/waxShader.vert.spv", "spv/waxShader.frag.spv", waxDescriptorSet->GetDescriptorSetLayoutPtr()));

	DescriptorSet* blendingWeightDescriptor = dynamic_cast<DescriptorSet*>(FindObjectByName("blendingWeightDescriptor"));
	graphicResources.push_back(new Pipeline(graphics, "blendingWeightPipeline", "spv/blendingWeight.vert.spv", "spv/blendingWeight.frag.spv", Vertex::GetBindingDescription(), Vertex::GetAttributeDescriptions(), sizeof(int), VK_SHADER_STAGE_VERTEX_BIT, blendingWeightDescriptor->GetDescriptorSetLayoutPtr()));

	graphicResources.push_back(new Pipeline(graphics, "vertexPipeline", "spv/vertexPoints.vert.spv", "spv/vertexPoints.frag.spv", Vertex::GetBindingDescription(), Vertex::GetAttributeDescriptions(), sizeof(VertexPipelinePushConstants), VK_SHADER_STAGE_VERTEX_BIT, blendingWeightDescriptor->GetDescriptorSetLayoutPtr(), VK_PRIMITIVE_TOPOLOGY_POINT_LIST));

	graphicResources.push_back(new Pipeline(graphics, "linePipeline", "spv/skeleton.vert.spv", "spv/skeleton.frag.spv", LineVertex::GetBindingDescription(), LineVertex::GetAttributeDescriptions(), sizeof(int), VK_SHADER_STAGE_VERTEX_BIT, blendingWeightDescriptor->GetDescriptorSetLayoutPtr(), VK_PRIMITIVE_TOPOLOGY_LINE_LIST, VK_FALSE));

	InitUniformBufferData();

	hairBone0 = new HairBone("HairBone");
	graphicResources.push_back(new Buffer(graphics, std::string("HairBone"), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(glm::vec3), hairBone0->GetBoneSize(), hairBone0->GetBoneData()));
	graphicResources.push_back(new UniformBuffer(graphics, std::string("HairBoneUniform"), hairBone0->GetHairBoneMaxDataSize(), Graphics::MAX_FRAMES_IN_FLIGHT));
	DescriptorSet* hairBoneDescriptor = new DescriptorSet(graphics, "hairBoneDescriptor", Graphics::MAX_FRAMES_IN_FLIGHT,
		{
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
			{1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
			{2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
			{3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}
		});
	graphicResources.push_back(hairBoneDescriptor);
	WriteHairBoneDescriptorSet();
	graphicResources.push_back(new Pipeline(graphics, "hairBonePipeline", "spv/hairBone.vert.spv", "spv/hairBone.frag.spv", LineVertex::GetBindingDescription(), LineVertex::GetAttributeDescriptions(), sizeof(HairBonePushConstants), VK_SHADER_STAGE_VERTEX_BIT, hairBoneDescriptor->GetDescriptorSetLayoutPtr(), VK_PRIMITIVE_TOPOLOGY_POINT_LIST, VK_FALSE));

	return true;
}

void MyScene::CleanScene()
{

	for (Object* obj : graphicResources)
	{
		delete obj;
	}

	delete hairBone0;
	delete model;
}

void MyScene::DrawFrame(float dt, VkCommandBuffer commandBuffer, uint32_t currentFrameID)
{
	ModifyBone();

	UpdateTimer(dt);

	UpdateUniformBuffer(currentFrameID);

	UpdateAnimationUniformBuffer(currentFrameID);

	UpdateHairBoneBuffer(currentFrameID);

	RecordDrawModelCalls(commandBuffer);

	RecordDrawSkeletonCall(commandBuffer);

	RecordDrawHairBoneCall(commandBuffer);
}

void MyScene::FillBufferWithFloats(VkCommandBuffer cmdBuffer, VkBuffer dstBuffer, VkDeviceSize offset, VkDeviceSize size, const float value)
{
	vkCmdFillBuffer(cmdBuffer, dstBuffer, offset, size, *(const uint32_t*)&value);
}

void MyScene::LoadNewModel()
{
	windowHolder->isPathDropped = false;
	const char* newPath = windowHolder->path;

	const int oldTextureSize = static_cast<const int>(model->GetDiffuseImagePaths().size());
	const int oldMeshSize = model->GetMeshSize();

	if (model->LoadModel(newPath) == false)
	{
		windowHolder->DisplayMessage("Failed model loading!", model->GetErrorString());
		return;
	}

	// In order to clean previous model buffers successfully, 
			// I should guarantee that deleted buffers are not in use by a command buffer.
	// Thus, wait until the submitted command buffer completed execution.
	graphics->DeviceWaitIdle();

	// Reload textures
	const std::vector<std::string> texturePaths = model->GetDiffuseImagePaths();
	const int textureSize = static_cast<const int>(texturePaths.size());
	for (int i = 0; i < textureSize; i++)
	{
		if (i < oldTextureSize)
		{
			Texture* texture = dynamic_cast<Texture*>(FindObjectByName(std::string("diffuseImage") + std::to_string(i)));
			texture->ChangeTexture(texturePaths[i]);
		}
		else
		{
			graphicResources.push_back(new Texture(graphics, std::string("diffuseImage") + std::to_string(i), texturePaths[i]));
		}
	}
	for (int i = textureSize; i < oldTextureSize; i++)
	{
		Object* WillBeDeleted = FindObjectByName(std::string("diffuseImage") + std::to_string(i));
		const auto& iter = std::find(graphicResources.begin(), graphicResources.end(), WillBeDeleted);
		graphicResources.erase(iter);
		delete WillBeDeleted;
	}

	// Reload model buffers
	const int meshSize = model->GetMeshSize();
	for (int i = 0; i < meshSize; i++)
	{
		if (i < oldMeshSize)
		{
			Buffer* buffer = dynamic_cast<Buffer*>(FindObjectByName(std::string("vertex") + std::to_string(i)));
			buffer->ChangeBufferData(sizeof(Vertex), model->GetVertexCount(i), model->GetVertexData(i));
			Buffer* indexBuffer = dynamic_cast<Buffer*>(FindObjectByName(std::string("index") + std::to_string(i)));
			indexBuffer->ChangeBufferData(sizeof(uint32_t), model->GetIndexCount(i), model->GetIndexData(i));
			Buffer* uniqueBuffer = dynamic_cast<Buffer*>(FindObjectByName(std::string("uniqueVertex") + std::to_string(i)));
			uniqueBuffer->ChangeBufferData(sizeof(Vertex), model->GetUniqueVertexCount(i), model->GetUniqueVertexData(i));
		}
		else
		{
			graphicResources.push_back(new Buffer(graphics, std::string("vertex") + std::to_string(i), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(Vertex), model->GetVertexCount(i), model->GetVertexData(i)));
			graphicResources.push_back(new Buffer(graphics, std::string("index") + std::to_string(i), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, sizeof(uint32_t), model->GetIndexCount(i), model->GetIndexData(i)));
			graphicResources.push_back(new Buffer(graphics, std::string("uniqueVertex") + std::to_string(i), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(Vertex), model->GetUniqueVertexCount(i), model->GetUniqueVertexData(i)));
		}
	}
	for (int i = meshSize; i < oldMeshSize; i++)
	{
		Object* WillBeDeleted = FindObjectByName(std::string("vertex") + std::to_string(i));
		const auto& iter = std::find(graphicResources.begin(), graphicResources.end(), WillBeDeleted);
		graphicResources.erase(iter);
		delete WillBeDeleted;

		Object* WillBeDeleted2 = FindObjectByName(std::string("index") + std::to_string(i));
		const auto& iter2 = std::find(graphicResources.begin(), graphicResources.end(), WillBeDeleted2);
		graphicResources.erase(iter2);
		delete WillBeDeleted2;

		Object* WillBeDeleted3 = FindObjectByName(std::string("uniqueVertex") + std::to_string(i));
		const auto& iter3 = std::find(graphicResources.begin(), graphicResources.end(), WillBeDeleted3);
		graphicResources.erase(iter3);
		delete WillBeDeleted3;
	}

	Buffer* skeletonBuffer = dynamic_cast<Buffer*>(FindObjectByName("skeletonBuffer"));
	skeletonBuffer->ChangeBufferData(sizeof(LineVertex), 2 * model->GetBoneCount(), model->GetBoneDataForDrawing());


	UniformBuffer* animationUniformBuffer = dynamic_cast<UniformBuffer*>(FindObjectByName("animationUniformBuffer"));
	animationUniformBuffer->ChangeBufferData(sizeof(glm::mat4) * model->GetBoneCount(), Graphics::MAX_FRAMES_IN_FLIGHT);

	UniformBuffer* unitBoneUniformBuffer = dynamic_cast<UniformBuffer*>(FindObjectByName("unitBoneUniformBuffer"));
	unitBoneUniformBuffer->ChangeBufferData(sizeof(glm::mat4) * model->GetBoneCount(), Graphics::MAX_FRAMES_IN_FLIGHT);

	MyImGUI::UpdateClickedVertexAddress(nullptr);
	MyImGUI::UpdateAnimationNameList();
	MyImGUI::UpdateBoneNameList();
	MyImGUI::UpdateMeshNameList();

	InitUniformBufferData();

	// If loaded data is waxing model(do not have texture data),
	if (textureSize <= 0)
	{
		if (meshSize > oldMeshSize)
		{
			DescriptorSet* waxDes = dynamic_cast<DescriptorSet*>(FindObjectByName("waxDescriptor"));
			waxDes->ChangeDescriptorSet(Graphics::MAX_FRAMES_IN_FLIGHT * meshSize, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
					{1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}
				});
			DescriptorSet* blendingDes = dynamic_cast<DescriptorSet*>(FindObjectByName("blendingWeightDescriptor"));
			blendingDes->ChangeDescriptorSet(Graphics::MAX_FRAMES_IN_FLIGHT * meshSize, {
				{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
				{1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}
				});

			DescriptorSet* hairBoneDescriptor = dynamic_cast<DescriptorSet*>(FindObjectByName("hairBoneDescriptor"));
			hairBoneDescriptor->ChangeDescriptorSet(Graphics::MAX_FRAMES_IN_FLIGHT,
				{
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
			{1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
			{2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
			{3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}
				});
		}
		WriteWaxDescriptorSet();
		WriteBlendingWeightDescriptorSet();
		WriteHairBoneDescriptorSet();

		return;
	}

	// It is an old code when there was no wax descriptor sets.
	// Currently, Create descriptor sets at everytime reloads, which might not be good. (?)
	// Solve two different functions, merge together and recover this functionality back.
	if (meshSize > oldMeshSize || textureSize > oldTextureSize)
	{
		DescriptorSet* des = dynamic_cast<DescriptorSet*>(FindObjectByName("descriptor"));
		des->ChangeDescriptorSet(Graphics::MAX_FRAMES_IN_FLIGHT * meshSize, {
		{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
		{1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
		{2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}
			});
		DescriptorSet* blendingDes = dynamic_cast<DescriptorSet*>(FindObjectByName("blendingWeightDescriptor"));
		blendingDes->ChangeDescriptorSet(Graphics::MAX_FRAMES_IN_FLIGHT * meshSize, {
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
			{1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}
			});

		DescriptorSet* hairBoneDescriptor = dynamic_cast<DescriptorSet*>(FindObjectByName("hairBoneDescriptor"));
		hairBoneDescriptor->ChangeDescriptorSet(Graphics::MAX_FRAMES_IN_FLIGHT,
			{
		{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
		{1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
		{2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
		{3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}
			});
	}
	WriteDescriptorSet();
	WriteBlendingWeightDescriptorSet();
	WriteHairBoneDescriptorSet();

}

void MyScene::InitGUI()
{
	MyImGUI::SendModelInfo(model, &showModel, &vertexPointsMode, &pointSize, &selectedMesh);
	MyImGUI::SendSkeletonInfo(&showSkeletonFlag, &blendingWeightMode, &selectedBone);
	MyImGUI::SendAnimationInfo(&timer, &bindPoseFlag);
	MyImGUI::SendConfigInfo(&mouseSensitivity);
	MyImGUI::SendHairBoneInfo(hairBone0, &applyingBone);

	MyImGUI::UpdateAnimationNameList();
	MyImGUI::UpdateBoneNameList();
	MyImGUI::UpdateMeshNameList();
}

void MyScene::UpdateTimer(float dt)
{
	timer += dt;

	if (timer > model->GetAnimationDuration())
	{
		timer = 0.f;
	}
}

void MyScene::RecordDrawModelCalls(VkCommandBuffer commandBuffer)
{

	const int meshSize = model->GetMeshSize();
	for (int i = 0; i < meshSize; i++)
	{
		Buffer* vertexBuffer = dynamic_cast<Buffer*>(FindObjectByName(std::string("vertex") + std::to_string(i)));
		Buffer* indexBuffer = dynamic_cast<Buffer*>(FindObjectByName(std::string("index") + std::to_string(i)));
		VkBuffer VB[] = { vertexBuffer->GetBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, VB, offsets);
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

		// If show model flag is on, display model and blending weight model
		if (showModel == true)
		{
			if (blendingWeightMode == true)
			{
				Pipeline* bwPipeline = dynamic_cast<Pipeline*>(FindObjectByName("blendingWeightPipeline"));
				DescriptorSet* bwDes = dynamic_cast<DescriptorSet*>(FindObjectByName("blendingWeightDescriptor"));
				RecordPushConstants(commandBuffer, bwPipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, &selectedBone, sizeof(int));

				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, bwPipeline->GetPipeline());
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, bwPipeline->GetPipelineLayout(), 0, 1, bwDes->GetDescriptorSetPtr(i * Graphics::MAX_FRAMES_IN_FLIGHT + graphics->GetCurrentFrameID()), 0, nullptr);
				vkCmdDrawIndexed(commandBuffer, indexBuffer->GetBufferDataSize(), 1, 0, 0, 0);
			}
			else
			{
				if (model->GetDiffuseImagePaths().size() <= 0)
				{
					Pipeline* wPipeline = dynamic_cast<Pipeline*>(FindObjectByName("waxPipeline"));
					DescriptorSet* wDes = dynamic_cast<DescriptorSet*>(FindObjectByName("waxDescriptor"));
					vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, wPipeline->GetPipeline());
					vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, wPipeline->GetPipelineLayout(), 0, 1, wDes->GetDescriptorSetPtr(i * Graphics::MAX_FRAMES_IN_FLIGHT + graphics->GetCurrentFrameID()), 0, nullptr);
					vkCmdDrawIndexed(commandBuffer, indexBuffer->GetBufferDataSize(), 1, 0, 0, 0);
				}
				else
				{
					Pipeline* pipeline = dynamic_cast<Pipeline*>(FindObjectByName("pipeline"));
					DescriptorSet* des = dynamic_cast<DescriptorSet*>(FindObjectByName("descriptor"));
					vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipeline());
					vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipelineLayout(), 0, 1, des->GetDescriptorSetPtr(i * Graphics::MAX_FRAMES_IN_FLIGHT + graphics->GetCurrentFrameID()), 0, nullptr);
					vkCmdDrawIndexed(commandBuffer, indexBuffer->GetBufferDataSize(), 1, 0, 0, 0);
				}
			}
		}

		// No matter showing model or not, display vertex points if and only if vertex points mode is on.
		if (vertexPointsMode == true && ((selectedMesh == i) || selectedMesh == meshSize))
		{
			Buffer* uniqueBuffer = dynamic_cast<Buffer*>(FindObjectByName(std::string("uniqueVertex") + std::to_string(i)));
			VkBuffer uniqueVB[] = { uniqueBuffer->GetBuffer() };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, uniqueVB, offsets);
			Pipeline* vPipeline = dynamic_cast<Pipeline*>(FindObjectByName("vertexPipeline"));
			DescriptorSet* des = dynamic_cast<DescriptorSet*>(FindObjectByName("blendingWeightDescriptor"));

			VertexPipelinePushConstants tmp;
			tmp.pointSize = pointSize;
			tmp.vertexID = GetSelectedVertexID(selectedMesh);
			tmp.isMousePressed = input.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_1);

			if (
				input.IsMouseButtonTriggered(GLFW_MOUSE_BUTTON_1) &&
				(MyImGUI::IsMouseOnImGUIWindow() == false)
				)
			{
				if (tmp.vertexID != -1)
				{
					Vertex* vertices = reinterpret_cast<Vertex*>(model->GetUniqueVertexData(selectedMesh));
					MyImGUI::UpdateClickedVertexAddress(&vertices[tmp.vertexID]);
				}
				else
				{
					MyImGUI::UpdateClickedVertexAddress(nullptr);
				}
			}

			RecordPushConstants(commandBuffer, vPipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, &tmp, sizeof(VertexPipelinePushConstants));
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vPipeline->GetPipeline());
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vPipeline->GetPipelineLayout(), 0, 1, des->GetDescriptorSetPtr(i * Graphics::MAX_FRAMES_IN_FLIGHT + graphics->GetCurrentFrameID()), 0, nullptr);
			vkCmdDraw(commandBuffer, uniqueBuffer->GetBufferDataSize(), 1, 0, 0);
		}
	}
}

void MyScene::InitUniformBufferData()
{
	uniformData.model = model->CalculateAdjustBoundingBoxMatrix();
	uniformData.view = glm::lookAt(cameraPoint, targetPoint, glm::vec3(0.f, 1.f, 0.f));
	VkExtent2D swapchainExtent = graphics->GetSwapchainExtent();
	uniformData.proj = glm::perspective(glm::radians(45.f), swapchainExtent.width / static_cast<float>(swapchainExtent.height), 0.1f, 10.f);
	// flip the sign of the element because GLM originally designed for OpenGL, where Y coordinate of the clip coorinates is inverted.
	uniformData.proj[1][1] *= -1;
}

void MyScene::UpdateUniformBuffer(uint32_t currentFrameID)
{
	// Update camera if and only if the user does not control ImGui window.
	if (MyImGUI::IsMouseOnImGUIWindow() == false)
	{

		const bool isMousePressed = input.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_1);
		const bool isLeftAltPressed = input.IsKeyPressed(GLFW_KEY_LEFT_ALT);
		glm::vec3 view = glm::normalize(targetPoint - cameraPoint);
		glm::vec2 mouseDelta = glm::vec2(input.GetMousePosition() - input.GetPresentMousePosition()) * 0.01f * mouseSensitivity;
		// Move mouse during pressing left alt, move target position
		if (isMousePressed && isLeftAltPressed)
		{
			static constexpr glm::vec3 globalUp = glm::vec3(0.f, 1.f, 0.f);


			glm::vec3 newX = glm::normalize(glm::cross(globalUp, view));
			glm::vec3 newY = glm::cross(newX, view);

			const glm::vec3 delta = (newX * mouseDelta.x + newY * mouseDelta.y) * 0.01f;
			targetPoint += delta;
			cameraPoint += delta;

			view = glm::normalize(targetPoint - cameraPoint);
		}
		// Press wheel button, init target position
		else if (input.IsMouseButtonTriggered(GLFW_MOUSE_BUTTON_MIDDLE))
		{
			targetPoint = glm::vec3(0.f, 0.f, 0.f);
			cameraPoint = glm::vec3(0.f, 0.f, 2.f);
		}

		// Move mouse, rotate a model
		if (isMousePressed && !isLeftAltPressed)
		{
			uniformData.model = glm::rotate(glm::mat4(1.f), mouseDelta.x * glm::radians(1.f), glm::vec3(0.0f, 1.f, 0.f)) *
				glm::rotate(glm::mat4(1.f), -mouseDelta.y * glm::radians(1.f), glm::vec3(1.f, 0.f, 0.f)) *
				uniformData.model;
		}

		if (input.IsMouseButtonTriggered(GLFW_MOUSE_BUTTON_2))
		{
			rightMouseCenter = glm::vec3(input.GetMousePosition(), 0);
		}
		else if (input.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_2))
		{
			glm::vec3 mousePosition = glm::vec3(input.GetMousePosition(), 0) - rightMouseCenter;
			glm::vec3 previousPosition = glm::vec3(input.GetPresentMousePosition(), 0) - rightMouseCenter;

			glm::vec3 cross = glm::cross(mousePosition, previousPosition);
			float result = 0;
			if (abs(cross.z) >= std::numeric_limits<float>().epsilon())
			{
				result = (cross.z / abs(cross.z)) * glm::length(mouseDelta) * 0.45f;
			}

			uniformData.model = glm::rotate(glm::mat4(1.f), result * glm::radians(1.f), view) *
				uniformData.model;
		}

		// Zoom in & out
		cameraPoint = cameraPoint + (static_cast<float>(input.MouseWheelScroll()) * (targetPoint - cameraPoint) * 0.1f);
		uniformData.view = glm::lookAt(cameraPoint, targetPoint, glm::vec3(0.f, 1.f, 0.f));

		VkExtent2D swapchainExtent = graphics->GetSwapchainExtent();
		uniformData.proj = glm::perspective(glm::radians(45.f), swapchainExtent.width / static_cast<float>(swapchainExtent.height), 0.1f, 10.f);
		// flip the sign of the element because GLM originally designed for OpenGL, where Y coordinate of the clip coorinates is inverted.
		uniformData.proj[1][1] *= -1;
	}

	UniformBuffer* uniformBuffer = dynamic_cast<UniformBuffer*>(FindObjectByName("uniformBuffer"));


	void* data;
	vkMapMemory(graphics->GetDevice(), uniformBuffer->GetBufferMemory(currentFrameID), 0, uniformBuffer->GetBufferSize(), 0, &data);
	memcpy(data, &uniformData, uniformBuffer->GetBufferSize());
	vkUnmapMemory(graphics->GetDevice(), uniformBuffer->GetBufferMemory(currentFrameID));
}

glm::vec3 MyScene::GetMousePositionInWorldSpace(float targetZ)
{
	glm::vec3 projectionVector = GetProjectionVectorFromCamera();

	float t = (targetZ - cameraPoint.z) / projectionVector.z;
	glm::vec3 result = cameraPoint + t * projectionVector;

	return result;
}

glm::vec3 MyScene::GetProjectionVectorFromCamera()
{
	glm::ivec2 rawPosition = input.GetMousePosition();
	glm::ivec2 windowSize = windowHolder->GetWindowSize();
	glm::vec4 mousePosition{ static_cast<float>(rawPosition.x) / (windowSize.x / 2.f), static_cast<float>(rawPosition.y) / (windowSize.y / 2.f), 0.f, 1.f };

	// Flip temporarily to get top == positive y coordinate value
	uniformData.proj[1][1] *= -1;

	glm::vec4 projectedPosition = glm::inverse(uniformData.view) * glm::inverse(uniformData.proj) * mousePosition;
	projectedPosition /= projectedPosition.w;
	glm::vec3 projectionVector{ projectedPosition.x - cameraPoint.x, projectedPosition.y - cameraPoint.y, projectedPosition.z - cameraPoint.z };

	// Restore projection matrix
	uniformData.proj[1][1] *= -1;

	return projectionVector;
}

void MyScene::WriteDescriptorSet()
{
	DescriptorSet* descriptorSet = dynamic_cast<DescriptorSet*>(FindObjectByName("descriptor"));
	UniformBuffer* uniformBuffer = dynamic_cast<UniformBuffer*>(FindObjectByName("uniformBuffer"));
	UniformBuffer* animationUniformBuffer = dynamic_cast<UniformBuffer*>(FindObjectByName("animationUniformBuffer"));
	Texture* emergencyTexture = dynamic_cast<Texture*>(FindObjectByName("EmergencyTexture"));

	const std::vector<std::string>& paths = model->GetDiffuseImagePaths();
	std::vector<Texture*> textures(paths.size(), nullptr);
	for (int i = 0; i < paths.size(); i++)
	{
		if (Texture* image = dynamic_cast<Texture*>(FindObjectByName(std::string("diffuseImage") + std::to_string(i)));
			image != nullptr)
		{
			textures[i] = image;
		}
	}

	for (int i = 0; i < model->GetMeshSize(); i++)
	{
		for (int j = 0; j < Graphics::MAX_FRAMES_IN_FLIGHT; j++)
		{
			descriptorSet->Write(i * Graphics::MAX_FRAMES_IN_FLIGHT + j, 0, uniformBuffer->GetBuffer(j), uniformBuffer->GetBufferSize());
			descriptorSet->Write(i * Graphics::MAX_FRAMES_IN_FLIGHT + j, 1, animationUniformBuffer->GetBuffer(j), animationUniformBuffer->GetBufferSize());
			if (i < paths.size())
			{
				descriptorSet->Write(i * Graphics::MAX_FRAMES_IN_FLIGHT + j, 2, textures[i]->GetImageView(), graphics->GetTextureSampler());
			}
			else
			{
				descriptorSet->Write(i * Graphics::MAX_FRAMES_IN_FLIGHT + j, 2, emergencyTexture->GetImageView(), graphics->GetTextureSampler());
			}
		}
	}
}

void MyScene::WriteWaxDescriptorSet()
{
	DescriptorSet* descriptorSet = dynamic_cast<DescriptorSet*>(FindObjectByName("waxDescriptor"));
	UniformBuffer* uniformBuffer = dynamic_cast<UniformBuffer*>(FindObjectByName("uniformBuffer"));
	UniformBuffer* animationUniformBuffer = dynamic_cast<UniformBuffer*>(FindObjectByName("animationUniformBuffer"));
	for (int i = 0; i < model->GetMeshSize(); i++)
	{
		for (int j = 0; j < Graphics::MAX_FRAMES_IN_FLIGHT; j++)
		{
			descriptorSet->Write(i * Graphics::MAX_FRAMES_IN_FLIGHT + j, 0, uniformBuffer->GetBuffer(j), uniformBuffer->GetBufferSize());
			descriptorSet->Write(i * Graphics::MAX_FRAMES_IN_FLIGHT + j, 1, animationUniformBuffer->GetBuffer(j), animationUniformBuffer->GetBufferSize());
		}
	}
}

void MyScene::UpdateAnimationUniformBuffer(uint32_t currentFrameID)
{
	if (model->GetBoneCount() <= 0)
	{
		return;
	}

	std::vector<glm::mat4> animationBufferData;

	// Get animation key frame data
	model->GetAnimationData(timer, animationBufferData, bindPoseFlag);


	UniformBuffer* animationUniformBuffer = dynamic_cast<UniformBuffer*>(FindObjectByName("animationUniformBuffer"));
	void* data;
	vkMapMemory(graphics->GetDevice(), animationUniformBuffer->GetBufferMemory(currentFrameID), 0, animationUniformBuffer->GetBufferSize(), 0, &data);
	memcpy(data, animationBufferData.data(), animationUniformBuffer->GetBufferSize());
	vkUnmapMemory(graphics->GetDevice(), animationUniformBuffer->GetBufferMemory(currentFrameID));


	model->GetUnitBoneData(animationBufferData);
	UniformBuffer* unitBoneUniformBuffer = dynamic_cast<UniformBuffer*>(FindObjectByName("unitBoneUniformBuffer"));
	vkMapMemory(graphics->GetDevice(), unitBoneUniformBuffer->GetBufferMemory(currentFrameID), 0, unitBoneUniformBuffer->GetBufferSize(), 0, &data);
	memcpy(data, animationBufferData.data(), unitBoneUniformBuffer->GetBufferSize());
	vkUnmapMemory(graphics->GetDevice(), unitBoneUniformBuffer->GetBufferMemory(currentFrameID));
}

void MyScene::WriteBlendingWeightDescriptorSet()
{
	DescriptorSet* descriptorSet = dynamic_cast<DescriptorSet*>(FindObjectByName("blendingWeightDescriptor"));
	UniformBuffer* uniformBuffer = dynamic_cast<UniformBuffer*>(FindObjectByName("uniformBuffer"));
	UniformBuffer* animationUniformBuffer = dynamic_cast<UniformBuffer*>(FindObjectByName("animationUniformBuffer"));
	for (int i = 0; i < model->GetMeshSize(); i++)
	{
		for (int j = 0; j < Graphics::MAX_FRAMES_IN_FLIGHT; j++)
		{
			descriptorSet->Write(i * Graphics::MAX_FRAMES_IN_FLIGHT + j, 0, uniformBuffer->GetBuffer(j), uniformBuffer->GetBufferSize());
			descriptorSet->Write(i * Graphics::MAX_FRAMES_IN_FLIGHT + j, 1, animationUniformBuffer->GetBuffer(j), animationUniformBuffer->GetBufferSize());
		}
	}
}

void MyScene::RecordPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlagBits targetStage, void* data, uint32_t dataSize)
{
	vkCmdPushConstants(commandBuffer, layout, targetStage, 0, dataSize, data);
}

void MyScene::RecordPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlagBits targetStage, void const* data, uint32_t dataSize)
{
	vkCmdPushConstants(commandBuffer, layout, targetStage, 0, dataSize, data);
}

void MyScene::WriteHairBoneDescriptorSet()
{
	DescriptorSet* hairBoneDescriptor = dynamic_cast<DescriptorSet*>(FindObjectByName("hairBoneDescriptor"));
	UniformBuffer* uniformBuffer = dynamic_cast<UniformBuffer*>(FindObjectByName("uniformBuffer"));
	UniformBuffer* animationUniformBuffer = dynamic_cast<UniformBuffer*>(FindObjectByName("animationUniformBuffer"));
	UniformBuffer* unitBoneUniformBuffer = dynamic_cast<UniformBuffer*>(FindObjectByName("unitBoneUniformBuffer"));
	UniformBuffer* uniformBuffer2 = dynamic_cast<UniformBuffer*>(FindObjectByName("HairBoneUniform"));

	for (int j = 0; j < Graphics::MAX_FRAMES_IN_FLIGHT; j++)
	{
		hairBoneDescriptor->Write(j, 0, uniformBuffer->GetBuffer(j), uniformBuffer->GetBufferSize());
		hairBoneDescriptor->Write(j, 1, animationUniformBuffer->GetBuffer(j), animationUniformBuffer->GetBufferSize());
		hairBoneDescriptor->Write(j, 2, unitBoneUniformBuffer->GetBuffer(j), unitBoneUniformBuffer->GetBufferSize());
		hairBoneDescriptor->Write(j, 3, uniformBuffer2->GetBuffer(j), uniformBuffer2->GetBufferSize());
	}
}

void MyScene::RecordDrawHairBoneCall(VkCommandBuffer commandBuffer)
{
	if (selectedMesh >= model->GetMeshSize() || selectedMesh < 0)
	{
		return;
	}
	Buffer* vertexBuffer = dynamic_cast<Buffer*>(FindObjectByName(std::string("HairBone")));
	VkBuffer VB[] = { vertexBuffer->GetBuffer() };
	VkDeviceSize offsets[] = { 0 };
	Pipeline* pipeline = dynamic_cast<Pipeline*>(FindObjectByName("hairBonePipeline"));
	DescriptorSet* des = dynamic_cast<DescriptorSet*>(FindObjectByName("hairBoneDescriptor"));
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, VB, offsets);
	HairBonePushConstants hpc{ pointSize, selectedBone };
	RecordPushConstants(commandBuffer, pipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, &hpc, sizeof(HairBonePushConstants));
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipeline());
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipelineLayout(), 0, 1, des->GetDescriptorSetPtr(graphics->GetCurrentFrameID()), 0, nullptr);
	vkCmdDraw(commandBuffer, 1, hairBone0->GetBoneSize(), 0, 0);
}

void MyScene::UpdateHairBoneBuffer(uint32_t currentFrameID)
{
	if (selectedMesh < 0 || selectedMesh >= model->GetMeshSize() || hairBone0->GetBoneSize() <= 0)
	{
		return;
	}
	UniformBuffer* uniformBuffer = dynamic_cast<UniformBuffer*>(FindObjectByName("HairBoneUniform"));
	void* data;
	vkMapMemory(graphics->GetDevice(), uniformBuffer->GetBufferMemory(currentFrameID), 0, hairBone0->GetHairBoneMaxDataSize(), 0, &data);
	memcpy(data, hairBone0->GetBoneData(), hairBone0->GetHairBoneMaxDataSize());
	vkUnmapMemory(graphics->GetDevice(), uniformBuffer->GetBufferMemory(currentFrameID));
}

Object* MyScene::FindObjectByName(std::string name)
{
	for (Object* obj : graphicResources)
	{
		if (obj->GetName().compare(name) == 0)
			return obj;
	}
	return nullptr;
}

void MyScene::ModifyBone()
{
	if (applyingBone == false)
	{
		return;
	}
	applyingBone = false;
	
	static int newBoneIndex = 0;
	// Modify Bone at here.
	Bone newBone;
	newBone.name = std::string("newBone" + std::to_string(newBoneIndex++));
	newBone.id = model->GetBoneCount();
	newBone.parentID = selectedBone;
	const Bone& parentBone = model->GetBone(selectedBone);
	glm::vec4 trans = hairBone0->GetBoneData(0);
	newBone.toBoneFromUnit = glm::translate(glm::vec3(trans.x, trans.y, trans.z)) * parentBone.toBoneFromUnit;
	newBone.toModelFromBone = parentBone.toModelFromBone;
	/*
	@@ TODO: Find out which one is correct
	newBone.toModelFromBone = parentBone.toModelFromBone /** glm::translate(-glm::vec3(trans.x, trans.y, trans.z));
	*/
	model->AddBone(newBone);

	MyImGUI::UpdateBoneNameList();

	graphics->DeviceWaitIdle();
	
	Buffer* skeleton = dynamic_cast<Buffer*>(FindObjectByName("skeletonBuffer"));
	skeleton->ChangeBufferData(sizeof(LineVertex), 2 * model->GetBoneCount(), model->GetBoneDataForDrawing());

	UniformBuffer* animation = dynamic_cast<UniformBuffer*>(FindObjectByName("animationUniformBuffer"));
	animation->ChangeBufferData(sizeof(glm::mat4) * model->GetBoneCount(), Graphics::MAX_FRAMES_IN_FLIGHT);

	UniformBuffer* unitBoneUniformBuffer = dynamic_cast<UniformBuffer*>(FindObjectByName("unitBoneUniformBuffer"));
	unitBoneUniformBuffer->ChangeBufferData(sizeof(glm::mat4) * model->GetBoneCount(), Graphics::MAX_FRAMES_IN_FLIGHT);
	WriteDescriptorSet();
	WriteWaxDescriptorSet();
	WriteBlendingWeightDescriptorSet();
	WriteHairBoneDescriptorSet();

	hairBone0->SetBoneData(0, glm::vec4(0.f, 0.f, 0.f, 1.f));
}

bool MyScene::HasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void MyScene::RecordDrawSkeletonCall(VkCommandBuffer commandBuffer)
{
	const int boneSize = model->GetBoneCount();
	if (boneSize <= 0 || showSkeletonFlag == false)
	{
		return;
	}

	Pipeline* linePipeline = dynamic_cast<Pipeline*>(FindObjectByName("linePipeline"));
	if (blendingWeightMode == true)
	{
		RecordPushConstants(commandBuffer, linePipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, &selectedBone, sizeof(int));
	}
	else
	{
		const int noData{ -1 };
		RecordPushConstants(commandBuffer, linePipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, &noData, sizeof(int));
	}

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, linePipeline->GetPipeline());

	Buffer* skeletonBuffer = dynamic_cast<Buffer*>(FindObjectByName("skeletonBuffer"));
	DescriptorSet* bwDescriptor = dynamic_cast<DescriptorSet*>(FindObjectByName("blendingWeightDescriptor"));
	VkBuffer VB[] = { skeletonBuffer->GetBuffer() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, VB, offsets);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, linePipeline->GetPipelineLayout(), 0, 1, bwDescriptor->GetDescriptorSetPtr(graphics->GetCurrentFrameID()), 0, nullptr);

	vkCmdDraw(commandBuffer, boneSize * 2, 1, 0, 0);
}

int MyScene::GetSelectedVertexID(int selectedMesh)
{
	if (selectedMesh == model->GetMeshSize())
	{
		return -1;
	}
	glm::vec3 projectionVector = GetProjectionVectorFromCamera();

	int dataSize = model->GetUniqueVertexCount(selectedMesh);
	Vertex* vertices = reinterpret_cast<Vertex*>(model->GetUniqueVertexData(selectedMesh));
	for (int i = 0; i < dataSize; i++)
	{
		glm::vec4 tmp = uniformData.model * glm::vec4(vertices[i].position.x, vertices[i].position.y, vertices[i].position.z, 1.f);
		glm::vec3 position(tmp.x, tmp.y, tmp.z);

		float t = (position.z - cameraPoint.z) / projectionVector.z;
		glm::vec3 projectedPoint = cameraPoint + t * projectionVector;
		glm::vec3 diff(projectedPoint - position);
		float distance = glm::dot(diff, diff);

		if (distance <= std::numeric_limits<float>::epsilon() * 15 * pointSize)
		{
			return i;
		}
	}

	return -1;
}
