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

MyScene::MyScene(Window* window)
	: windowHolder(window), meshSize(-1), model(nullptr), timer(0.f), rightMouseCenter(glm::vec3(0.f, 0.f, 0.f)), cameraPoint(glm::vec3(0.f, 2.f, 2.f)), targetPoint(glm::vec3(0.f)), boneSize(0), animationCount(0), animationUniformBufferSize(0), bindPoseFlag(false), showSkeletonFlag(true), blendingWeightMode(false), showModel(true), vertexPointsMode(false), pointSize(5.f)
{
}

bool MyScene::InitScene()
{
	model = new Model("../Vulkan/Graphics/Model/models/Sitting Laughing.fbx");
	model->SetAnimationIndex(0);



	CreateEmergencyTexture();
	CreateTextures(model->GetDiffuseImagePaths());
	CreateTextureSampler();
	CreateBuffers();
	CreateDescriptorSet();
	CreateWaxDescriptorSet();
	CreateBlendingWeightDescriptorSet();


	VkShaderModule vertModule = CreateShaderModule(readFile("spv/vertexShader.vert.spv"));
	VkShaderModule fragModule = CreateShaderModule(readFile("spv/fragShader.frag.spv"));
	CreateGraphicsPipeline(vertModule, fragModule, descriptorSet->GetDescriptorSetLayoutPtr(), pipeline, pipelineLayout);

	VkShaderModule waxVertModule = CreateShaderModule(readFile("spv/waxShader.vert.spv"));
	VkShaderModule waxFragModule = CreateShaderModule(readFile("spv/waxShader.frag.spv"));
	CreateGraphicsPipeline(waxVertModule, waxFragModule, waxDescriptorSet->GetDescriptorSetLayoutPtr(), waxPipeline, waxPipelineLayout);

	VkShaderModule blendingWeightVertModule = CreateShaderModule(readFile("spv/blendingWeight.vert.spv"));
	VkShaderModule blendingWeightFragModule = CreateShaderModule(readFile("spv/blendingWeight.frag.spv"));
	CreateGraphicsPipeline(blendingWeightVertModule, blendingWeightFragModule, sizeof(int), VK_SHADER_STAGE_VERTEX_BIT, blendingWeightDescriptorSet->GetDescriptorSetLayoutPtr(), blendingWeightPipeline, blendingWeightPipelineLayout);

	VkShaderModule vertexPointsVertModule = CreateShaderModule(readFile("spv/vertexPoints.vert.spv"));
	VkShaderModule vertexPointsFragModule = CreateShaderModule(readFile("spv/vertexPoints.frag.spv"));
																														 // Let's use waxDescriptor set because it can be used so far and to reduce memory allocations
	CreateGraphicsPipeline(vertexPointsVertModule, vertexPointsFragModule, sizeof(float), VK_SHADER_STAGE_VERTEX_BIT, blendingWeightDescriptorSet->GetDescriptorSetLayoutPtr(), vertexPointsPipeline, vertexPointsPipelineLayout, VK_PRIMITIVE_TOPOLOGY_POINT_LIST);

	// it is using blendingWeightDescriptorSet
	CreateLinePipeline();

	InitUniformBufferData();

	return true;
}

void MyScene::CleanScene()
{

	DestroyPipeline();

	DestroyTextureSampler();
	DestroyEmergencyTexture();

	DestroyDescriptorSet();
	DestroyWaxDescriptorSet();
	DestroyBlendingWeightDescriptorSet();

	DestroyBuffersAndFreeMemories();

	delete model;
}

void MyScene::DrawFrame(float dt)
{
	UpdateTimer(dt);

	// Synchronize with GPU
	vkWaitForFences(device, 1, &inFlightFences[currentFrameID], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult resultGetNextImage = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrameID], VK_NULL_HANDLE, &imageIndex);
	if (windowHolder->windowFramebufferResized || resultGetNextImage == VK_ERROR_OUT_OF_DATE_KHR)
	{
		windowHolder->SetWindowFramebufferResized(false);
		RecreateSwapchain();
		return;
	}
	else if (resultGetNextImage != VK_SUCCESS && resultGetNextImage != VK_SUBOPTIMAL_KHR)
	{
		std::cout << "Acquiring next image has failed!" << std::endl;
		abort();
	}

	UpdateUniformBuffer(currentFrameID);
	UpdateAnimationUniformBuffer();

	// Prevent deadlock, delay ResetFences
	vkResetFences(device, 1, &inFlightFences[currentFrameID]);

	vkResetCommandBuffer(commandBuffers[currentFrameID], 0);
	RecordCommandBuffer(commandBuffers[currentFrameID], imageIndex);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrameID] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[currentFrameID];

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrameID] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	VulkanHelper::VkCheck(vkQueueSubmit(queue, 1, &submitInfo, inFlightFences[currentFrameID]), "Submitting queue has failed!");

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapchains[] = { swapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = &imageIndex;
	// It allows you to specify an array of VkResult values to check for every individual swap chain if presentation was successful.
	// It's not necessary if you're only using a single swap chain, because you can simply use the return value of the present function.
	presentInfo.pResults = nullptr;

	VkResult resultQueuePresent = vkQueuePresentKHR(queue, &presentInfo);
	if (windowHolder->windowFramebufferResized || resultQueuePresent == VK_ERROR_OUT_OF_DATE_KHR || resultQueuePresent == VK_SUBOPTIMAL_KHR)
	{
		windowHolder->SetWindowFramebufferResized(false);
		RecreateSwapchain();
	}
	else if (resultQueuePresent != VK_SUCCESS)
	{
		std::cout << "Acquiring next image has failed!" << std::endl;
		abort();
	}

	UpdateCurrentFrameID();
}

void MyScene::CreateBuffers()
{
	CreateSkeletonBuffer();
	CreateModelBuffers();
	CreateUniformBuffers();
	CreateAnimationUniformBuffers();
}

void MyScene::ResizeModelBuffers(int size)
{
	vertexBuffers.resize(size);
	vertexBufferMemories.resize(size);
	indexBuffers.resize(size);
	indexBufferMemories.resize(size);
	indexCounts.resize(size);
}


double MyScene::linearTosRGB(double cl)
{
	double cs = 0.0;
	if (cl >= 1.0)
	{
		cs = 1.0;
	}
	else if (cl <= 0.0)
	{
		cs = 0.0;
	}
	else if (cl < 0.0031308)
	{
		cs = 12.92 * cl;
	}
	else
	{
		cs = 1.055 * pow(cl, 0.41666) - 0.55;
	}

	return cs;
}
double MyScene::sRGBToLinear(double cs)
{
	double cl = 0.0;
	if (cs >= 1.0)
	{
		cl = 1.0;
	}
	else if (cs <= 0.0)
	{
		cl = 0.0;
	}
	else if (cs <= 0.04045)
	{
		cl = cs / 12.92;
	}
	else
	{
		cl = pow(((cs + 0.0555) / 1.055), 2.4);
	}

	return cl;
}

void MyScene::FillBufferWithFloats(VkCommandBuffer cmdBuffer, VkBuffer dstBuffer, VkDeviceSize offset, VkDeviceSize size, const float value)
{
	vkCmdFillBuffer(cmdBuffer, dstBuffer, offset, size, *(const uint32_t*)&value);
}
void MyScene::LoadNewModel()
{
	windowHolder->isPathDropped = false;
	const char* newPath = windowHolder->path;

	if (model->LoadModel(newPath) == false)
	{
		windowHolder->DisplayMessage("Failed model loading!", model->GetErrorString());
		return;
	}

	// In order to clean previous model buffers successfully, 
			// I should guarantee that deleted buffers are not in use by a command buffer.
	// Thus, wait until the submitted command buffer completed execution.
	VulkanHelper::VkCheck(vkDeviceWaitIdle(device), "failed to make logical device idle");

	const int oldTextureViewSize = static_cast<const int>(textureImageViews.size());

	DestroyTextureImage();
	DestroyModelBuffers();
	DestroySkeletonBuffer();
	DestroyAnimationUniformBuffers();

	const int oldMeshSize = meshSize;

	MyImGUI::UpdateAnimationNameList();
	MyImGUI::UpdateBoneNameList();

	CreateSkeletonBuffer();
	CreateAnimationUniformBuffers();
	CreateModelBuffers();
	CreateTextures(model->GetDiffuseImagePaths());
	InitUniformBufferData();

	// If loaded data is waxing model(do not have texture data),
	if (textureImageViews.size() <= 0)
	{
		if (meshSize > oldMeshSize)
		{
			DestroyWaxDescriptorSet();
			DestroyBlendingWeightDescriptorSet();
			CreateWaxDescriptorSet();
			CreateBlendingWeightDescriptorSet();
		}
		else
		{
			WriteWaxDescriptorSet();
			WriteBlendingWeightDescriptorSet();
		}

		return;
	}

	// It is an old code when there was no wax descriptor sets.
	// Currently, Create descriptor sets at everytime, which might not be good.
	// Solve two different functions, merge together and recover this functionality back.
	if (meshSize > oldMeshSize || textureImageViews.size() > oldTextureViewSize)
	{
		DestroyDescriptorSet();
		DestroyBlendingWeightDescriptorSet();
		CreateDescriptorSet();
		CreateBlendingWeightDescriptorSet();
	}
	else
	{
		WriteDescriptorSet();
		WriteBlendingWeightDescriptorSet();
	}

}

void MyScene::InitGUI()
{
	MyImGUI::InitImGUI(windowHolder->glfwWindow, device, instance, physicalDevice, queue, renderPass, commandBuffers.front());

	MyImGUI::SendModelInfo(model, &showModel, &vertexPointsMode, &pointSize);
	MyImGUI::SendSkeletonInfo(&showSkeletonFlag, &blendingWeightMode, &selectedBone);
	MyImGUI::SendAnimationInfo(&timer, &bindPoseFlag);
	MyImGUI::UpdateAnimationNameList();
	MyImGUI::UpdateBoneNameList();
}

void MyScene::UpdateTimer(float dt)
{
	timer += dt;

	if (timer > model->GetAnimationDuration())
	{
		timer = 0.f;
	}
}

void MyScene::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;

	VulkanHelper::VkCheck(vkBeginCommandBuffer(commandBuffer, &beginInfo), "Begining command buffer has failed!");


	// Starting a render pass??????
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.framebuffer = swapchainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = swapchainExtent;

	// Since VkClearValue is union, use appropriate member variable name for usage.
	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { { 0.4f, 0.f, 0.f, 1.f} };
	clearValues[1].depthStencil = { 1.f, 0 };

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0.f;
	viewport.y = 0.f;
	viewport.width = static_cast<float>(swapchainExtent.width);
	viewport.height = static_cast<float>(swapchainExtent.height);
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapchainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);


	RecordDrawModelCalls(commandBuffer);

	RecordDrawSkeletonCall(commandBuffer);

	MyImGUI::GUIRender(commandBuffer);

	vkCmdEndRenderPass(commandBuffer);

	VulkanHelper::VkCheck(vkEndCommandBuffer(commandBuffer), "Ending command buffer has failed!");
}

void MyScene::RecordDrawModelCalls(VkCommandBuffer commandBuffer)
{
	// If show model flag is on, display model and blending weight model
	if (showModel == true)
	{
		if (blendingWeightMode == true)
		{
			RecordPushConstants(commandBuffer, blendingWeightPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, &selectedBone, sizeof(int));
			RecordDrawMeshCall(commandBuffer, blendingWeightPipeline, blendingWeightPipelineLayout, blendingWeightDescriptorSet);
		}
		else
		{
			if (textureImageViews.size() <= 0)
			{
				RecordDrawMeshCall(commandBuffer, waxPipeline, waxPipelineLayout, waxDescriptorSet);
			}
			else
			{
				RecordDrawMeshCall(commandBuffer, pipeline, pipelineLayout, descriptorSet);
			}
		}
	}

	// No matter showing model or not, display vertex points if and only if vertex points mode is on.
	if (vertexPointsMode == true)
	{
		RecordPushConstants(commandBuffer, vertexPointsPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, &pointSize, sizeof(float));
		RecordDrawMeshCall(commandBuffer, vertexPointsPipeline, vertexPointsPipelineLayout, blendingWeightDescriptorSet);
	}
}

void MyScene::UpdateCurrentFrameID()
{
	currentFrameID = (currentFrameID + 1) % MAX_FRAMES_IN_FLIGHT;
}

void MyScene::CreateVertexBuffer(VkDeviceSize bufferSize, void* vertexData, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	// Use two buffers.
	// One for writing vertex data, the other is actual vertex buffer which we cannot see and use(map) at CPU.
	// The reason why use two buffers is the buffer we can see at CPU is not a good buffer from the GPU side.

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	// Operate as glMapBuffer, glUnmapBuffer
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertexData, static_cast<size_t>(bufferSize));
	vkUnmapMemory(device, stagingBufferMemory);

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMemory);

	CopyBuffer(stagingBuffer, buffer, bufferSize);

	DestroyBuffer(stagingBuffer);
	FreeMemory(stagingBufferMemory);
}

void MyScene::DestroyBuffersAndFreeMemories()
{
	DestroySkeletonBuffer();
	DestroyModelBuffers();

	DestroyAnimationUniformBuffers();
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		DestroyBuffer(uniformBuffers[i]);
		FreeMemory(uniformBuffersMemory[i]);
	}
}

void MyScene::CreateModelBuffers()
{
	meshSize = model->GetMeshSize();

	ResizeModelBuffers(meshSize);

	for (int i = 0; i < meshSize; i++)
	{
		CreateVertexBuffer(sizeof(Vertex) * model->GetVertexCount(i), model->GetVertexData(i), vertexBuffers[i], vertexBufferMemories[i]);
		CreateIndexBuffer(model->GetIndexCount(i), model->GetIndexData(i), i);
	}
}

void MyScene::DestroyModelBuffers()
{
	int previousMeshSize = static_cast<int>(vertexBuffers.size());

	for (int i = 0; i < previousMeshSize; i++)
	{
		DestroyBuffer(vertexBuffers[i]);
		FreeMemory(vertexBufferMemories[i]);

		DestroyBuffer(indexBuffers[i]);
		FreeMemory(indexBufferMemories[i]);
	}
}

void MyScene::DestroyBuffer(VkBuffer& buffer)
{
	vkDestroyBuffer(device, buffer, nullptr);
}

void MyScene::FreeMemory(VkDeviceMemory memory)
{
	vkFreeMemory(device, memory, nullptr);
}

void MyScene::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer copyCommandBuffer = BeginSingleTimeCommands();

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(copyCommandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	EndSingleTimeCommands(copyCommandBuffer);
}

void MyScene::CreateIndexBuffer(int indexCount, void* indexData, int i)
{
	indexCounts[i] = indexCount;
	VkDeviceSize bufferSize = sizeof(int32_t) * indexCount;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indexData, bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		indexBuffers[i], indexBufferMemories[i]);

	CopyBuffer(stagingBuffer, indexBuffers[i], bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void MyScene::CreateUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);
	uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);


	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			uniformBuffers[i], uniformBuffersMemory[i]);
	}
}

void MyScene::InitUniformBufferData()
{
	uniformData.model = model->CalculateAdjustBoundingBoxMatrix();
	uniformData.view = glm::lookAt(cameraPoint, targetPoint, glm::vec3(0.f, 0.f, 1.f));
	uniformData.proj = glm::perspective(glm::radians(45.f), swapchainExtent.width / static_cast<float>(swapchainExtent.height), 0.1f, 10.f);
	// flip the sign of the element because GLM originally designed for OpenGL, where Y coordinate of the clip coorinates is inverted.
	uniformData.proj[1][1] *= -1;
}

void MyScene::UpdateUniformBuffer(uint32_t currentImage)
{
	// Update camera if and only if the user does not control ImGui window.
	if (MyImGUI::IsMouseOnImGUIWindow() == false)
	{

		const bool isMousePressed = input.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_1);
		const bool isLeftAltPressed = input.IsKeyPressed(GLFW_KEY_LEFT_ALT);
		glm::vec3 view = glm::normalize(targetPoint - cameraPoint);
		glm::vec2 mouseDelta = input.GetMousePosition() - input.GetPresentMousePosition();

		if (isMousePressed && isLeftAltPressed)
		{
			static constexpr glm::vec3 globalUp = glm::vec3(0.f, 0.f, 1.f);


			glm::vec3 newX = glm::normalize(glm::cross(globalUp, view));
			glm::vec3 newY = glm::cross(newX, view);

			const glm::vec3 delta = (newX * mouseDelta.x + newY * mouseDelta.y) * 0.01f;
			targetPoint += delta;
			cameraPoint += delta;

			glm::vec3 view = glm::normalize(targetPoint - cameraPoint);
		}
		else if (input.IsMouseButtonTriggered(GLFW_MOUSE_BUTTON_MIDDLE))
		{
			targetPoint = glm::vec3(0.f, 0.f, 0.f);
			cameraPoint = glm::vec3(0.f, 2.f, 2.f);
		}


		if (isMousePressed && !isLeftAltPressed)
		{

			uniformData.model = glm::rotate(glm::mat4(1.f), mouseDelta.x * glm::radians(1.f), glm::vec3(0.0f, view.z, -view.y)) *
				glm::rotate(glm::mat4(1.f), mouseDelta.y * glm::radians(1.f), glm::vec3(-view.y, view.x, 0.0f)) *
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


		cameraPoint = cameraPoint + (static_cast<float>(input.MouseWheelScroll()) * (targetPoint - cameraPoint) * 0.1f);
		uniformData.view = glm::lookAt(cameraPoint, targetPoint, glm::vec3(0.f, 0.f, 1.f));
	}

	void* data;
	vkMapMemory(device, uniformBuffersMemory[currentFrameID], 0, sizeof(UniformBufferObject), 0, &data);
	memcpy(data, &uniformData, sizeof(UniformBufferObject));
	vkUnmapMemory(device, uniformBuffersMemory[currentFrameID]);
}

void MyScene::CreateDescriptorSet()
{
	descriptorSet = new DescriptorSet(device, MAX_FRAMES_IN_FLIGHT * meshSize, {
		{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
		{1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
		{2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}
		});

	WriteDescriptorSet();
}

void MyScene::WriteDescriptorSet()
{
	for (int i = 0; i < meshSize; i++)
	{
		for (int j = 0; j < MAX_FRAMES_IN_FLIGHT; j++)
		{
			descriptorSet->Write(i * MAX_FRAMES_IN_FLIGHT + j, 0, uniformBuffers[j], sizeof(UniformBufferObject));
			descriptorSet->Write(i * MAX_FRAMES_IN_FLIGHT + j, 1, animationUniformBuffers[j], animationUniformBufferSize);
			if (i < textureImageViews.size())
			{
				descriptorSet->Write(i * MAX_FRAMES_IN_FLIGHT + j, 2, textureImageViews[i], textureSampler);
			}
			else
			{
				descriptorSet->Write(i * MAX_FRAMES_IN_FLIGHT + j, 2, emergencyTextureImageView, textureSampler);
			}
		}
	}
}

void MyScene::DestroyDescriptorSet()
{
	delete descriptorSet;
}

void MyScene::CreateWaxDescriptorSet()
{
	waxDescriptorSet = new DescriptorSet(device, MAX_FRAMES_IN_FLIGHT * meshSize, {
		{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
		{1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}
		});

	WriteWaxDescriptorSet();
}

void MyScene::WriteWaxDescriptorSet()
{
	for (int i = 0; i < meshSize; i++)
	{
		for (int j = 0; j < MAX_FRAMES_IN_FLIGHT; j++)
		{
			waxDescriptorSet->Write(i * MAX_FRAMES_IN_FLIGHT + j, 0, uniformBuffers[j], sizeof(UniformBufferObject));
			waxDescriptorSet->Write(i * MAX_FRAMES_IN_FLIGHT + j, 1, animationUniformBuffers[j], animationUniformBufferSize);
		}
	}
}

void MyScene::DestroyWaxDescriptorSet()
{
	delete waxDescriptorSet;
}

void MyScene::CreateTextureSampler()
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	// Anisotropy filter info
	if (physicalDeviceFeatures.features.samplerAnisotropy == VK_TRUE)
	{
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = physicalDeviceProperties.limits.maxSamplerAnisotropy;
	}
	else
	{
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = 1.f;
	}

	// [0, texWidth) vs [0, 1)
	samplerInfo.unnormalizedCoordinates = VK_FALSE;

	// below info is mainly used for percentage-closer filtering on shadow maps.
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.f;
	samplerInfo.minLod = 0.f;
	samplerInfo.maxLod = 0.f;

	VulkanHelper::VkCheck(vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler), "Creating sampler has failed!");
}

void MyScene::DestroyTextureSampler()
{
	vkDestroySampler(device, textureSampler, nullptr);
}

void MyScene::CreateLinePipeline()
{
	VkShaderModule vertModule = CreateShaderModule(readFile("spv/skeleton.vert.spv"));
	VkShaderModule fragModule = CreateShaderModule(readFile("spv/skeleton.frag.spv"));

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertModule;
	vertShaderStageInfo.pName = "main";
	// pSpecializationInfo which is an optional allows you to specify values for shader constants.

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};
	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
	dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	const VkVertexInputBindingDescription& bindingDescription = LineVertex::GetBindingDescription();
	const std::vector<VkVertexInputAttributeDescription>& attributeDescription = LineVertex::GetAttributeDescriptions();
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
	inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	// If it is true, then fragments that are beyond the near and far planes are clamped to them as opposed to discarding them.
	// It is useful in some special cases like shadow maps which requires enabling a GPU feature.
	rasterizer.depthClampEnable = VK_FALSE;
	// If it is true, then geometry never passes through the rasterizer stage.
	// It is basically disables any output to the framebuffer.
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	// It determines how fragments are generated for geometry.
	// Using VK_POLYGON_MODE_LINE, MODE_POINT requires enabling a GPU feature.
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_FALSE;
	depthStencil.depthWriteEnable = VK_FALSE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.f;
	depthStencil.maxDepthBounds = 1.f;
	depthStencil.stencilTestEnable = VK_FALSE;


	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	// Bellow settings are optional, unless blendEnable is VK_FALSE.
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkPushConstantRange pushConstantRange{};
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(int);
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = blendingWeightDescriptorSet->GetDescriptorSetLayoutPtr();
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

	VulkanHelper::VkCheck(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &linePipelineLayout), "Creating pipelineLayout has failed!");

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	// shader stages
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	// Fixed-function states
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicStateCreateInfo;
	// Pipeline layout
	pipelineInfo.layout = linePipelineLayout;
	// Render pass
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	// Below parameters are used for specify parent pipeline to handle derived multiple pipelines, which we don't use it here.
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	VulkanHelper::VkCheck(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &linePipeline), "Creating graphics pipeline has failed!");



	vkDestroyShaderModule(device, vertModule, nullptr);
	vkDestroyShaderModule(device, fragModule, nullptr);
}

void MyScene::CreateSkeletonBuffer()
{
	boneSize = static_cast<int>(model->GetBoneCount());

	if (boneSize <= 0)
	{
		return;
	}

	CreateVertexBuffer(sizeof(LineVertex) * (2 * boneSize), model->GetBoneDataForDrawing(), skeletonLineBuffer, skeletonLineBufferMemory);
}

void MyScene::DestroySkeletonBuffer()
{
	DestroyBuffer(skeletonLineBuffer);
	FreeMemory(skeletonLineBufferMemory);
}

void MyScene::CreateBlendingWeightDescriptorSet()
{
	blendingWeightDescriptorSet = new DescriptorSet(device, MAX_FRAMES_IN_FLIGHT * meshSize, {
		{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
		{1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}
		});

	WriteBlendingWeightDescriptorSet();
}

void MyScene::UpdateAnimationUniformBuffer()
{
	if (boneSize <= 0)
	{
		return;
	}

	std::vector<glm::mat4> animationBufferData;

	// Get animation key frame data
	model->GetAnimationData(timer, animationBufferData, bindPoseFlag);


	void* data;
	vkMapMemory(device, animationUniformBufferMemories[currentFrameID], 0, animationUniformBufferSize, 0, &data);
	memcpy(data, animationBufferData.data(), animationUniformBufferSize);
	vkUnmapMemory(device, animationUniformBufferMemories[currentFrameID]);
}

void MyScene::WriteBlendingWeightDescriptorSet()
{
	for (int i = 0; i < meshSize; i++)
	{
		for (int j = 0; j < MAX_FRAMES_IN_FLIGHT; j++)
		{
			blendingWeightDescriptorSet->Write(i * MAX_FRAMES_IN_FLIGHT + j, 0, uniformBuffers[j], sizeof(UniformBufferObject));
			blendingWeightDescriptorSet->Write(i * MAX_FRAMES_IN_FLIGHT + j, 1, animationUniformBuffers[j], animationUniformBufferSize);
		}
	}
}

void MyScene::DestroyBlendingWeightDescriptorSet()
{
	delete blendingWeightDescriptorSet;
}

void MyScene::CreateAnimationUniformBuffers()
{
	if (boneSize <= 0)
	{
		return;
	}

	animationUniformBufferSize = sizeof(glm::mat4) * boneSize;
	animationUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	animationUniformBufferMemories.resize(MAX_FRAMES_IN_FLIGHT);

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		CreateBuffer(animationUniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			animationUniformBuffers[i], animationUniformBufferMemories[i]);
	}
}

void MyScene::DestroyAnimationUniformBuffers()
{
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		DestroyBuffer(animationUniformBuffers[i]);
		FreeMemory(animationUniformBufferMemories[i]);
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

bool MyScene::HasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void MyScene::RecordDrawSkeletonCall(VkCommandBuffer commandBuffer)
{
	if (boneSize <= 0 || showSkeletonFlag == false)
	{
		return;
	}

	if (blendingWeightMode == true)
	{
		RecordPushConstants(commandBuffer, linePipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, &selectedBone, sizeof(int));
	}
	else
	{
		const int noData{ -1 };
		RecordPushConstants(commandBuffer, linePipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, &noData, sizeof(int));
	}

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, linePipeline);

	VkBuffer VB[] = { skeletonLineBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, VB, offsets);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, linePipelineLayout, 0, 1, blendingWeightDescriptorSet->GetDescriptorSetPtr(currentFrameID), 0, nullptr);

	vkCmdDraw(commandBuffer, boneSize * 2, 1, 0, 0);
}

void MyScene::RecordDrawMeshCall(VkCommandBuffer commandBuffer, VkPipeline pipeline, VkPipelineLayout pipelineLayout, DescriptorSet* descriptorSet)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	for (int i = 0; i < meshSize; i++)
	{
		VkBuffer VB[] = { vertexBuffers[i] };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, VB, offsets);
		vkCmdBindIndexBuffer(commandBuffer, indexBuffers[i], 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, descriptorSet->GetDescriptorSetPtr(i * MAX_FRAMES_IN_FLIGHT + currentFrameID), 0, nullptr);

		vkCmdDrawIndexed(commandBuffer, indexCounts[i], 1, 0, 0, 0);
	}
}

void MyScene::CreateEmergencyTexture()
{

	int texWidth, texHeight, texChannels;

	stbi_set_flip_vertically_on_load(true);
	stbi_uc* pixels = stbi_load("../Vulkan/Graphics/Model/EssentialImages/transparent.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	constexpr int RGBA = 4;
	VkDeviceSize imageSize = texWidth * texHeight * RGBA;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(device, stagingBufferMemory);

	stbi_image_free(pixels);

	CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, emergencyTextureImage, emergencyTextureImageMemory);

	TransitionImageLayout(emergencyTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	CopyBufferToImage(stagingBuffer, emergencyTextureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	TransitionImageLayout(emergencyTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
	// Finished creating texture image (create staging buffer, copy image data, copy buffer data to image buffer, clean staging buffer)

	// Create Image View
	emergencyTextureImageView = CreateImageView(emergencyTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}
void MyScene::DestroyEmergencyTexture()
{
	vkDestroyImageView(device, emergencyTextureImageView, nullptr);
	vkDestroyImage(device, emergencyTextureImage, nullptr);
	vkFreeMemory(device, emergencyTextureImageMemory, nullptr);
}