/******************************************************************************
Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   Structs.h
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 09.27.2022
	header file for custom structures.
******************************************************************************/
#pragma once
#include <vector>
#include "Vulkan/vulkan.h"
#include <GLMath.h>

struct TutorialVertex {
	TutorialVertex(glm::vec2 position, glm::vec3 color)
		:position(position), color(color)
	{}
	glm::vec2 position;
	glm::vec3 color;

	static const VkVertexInputBindingDescription& GetBindingDescription()
	{
		static VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(TutorialVertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static const std::vector<VkVertexInputAttributeDescription>& GetAttributeDescriptions()
	{
		static std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;		// two 32-bit floats
		attributeDescriptions[0].offset = offsetof(TutorialVertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;		// two 32-bit floats
		attributeDescriptions[1].offset = offsetof(TutorialVertex, color);



		return attributeDescriptions;
	}
};

struct Vertex {
	Vertex()
	{
		position = glm::vec3(0.f, 0.f, 0.f);
		normal = glm::vec3(0.f, 0.f, 0.f);
	}
	Vertex(glm::vec3 position, glm::vec3 normal)
		:position(position), normal(normal)
	{}

	Vertex(const Vertex& v)
		:position(v.position), normal(v.normal)
	{}

	Vertex(Vertex&& v)
		:position(v.position), normal(v.normal)
	{}
	Vertex& operator=(const Vertex& v)
	{
		position = v.position;
		normal = v.normal;
		return *this;
	}
	Vertex& operator=(Vertex&& v)
	{
		position = v.position;
		normal = v.normal;
		return *this;
	}


		glm::vec3 position;
		glm::vec3 normal;

	static const VkVertexInputBindingDescription& GetBindingDescription()
	{
		static VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static const std::vector<VkVertexInputAttributeDescription>& GetAttributeDescriptions()
	{
		static std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;		// three 32-bit floats
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;		// three 32-bit floats
		attributeDescriptions[1].offset = offsetof(Vertex, normal);

		return attributeDescriptions;
	}
};

struct UniformBufferObject {
	UniformBufferObject()
		: model(glm::mat4()), view(glm::mat4()), proj(glm::mat4())
	{}
	UniformBufferObject(glm::mat4 model, glm::mat4 view, glm::mat4 proj)
		: model(model), view(view), proj(proj)
	{}
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};