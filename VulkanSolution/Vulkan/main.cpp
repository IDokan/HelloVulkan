/******************************************************************************
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   main.cpp
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 06.08.2021
	main function.
******************************************************************************/
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "GLMath.h"

#include <iostream>

#include "Engines/Engine.h"
#include "Engines/Timer.h"

int main()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan Window", nullptr, nullptr);

	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::cout << extensionCount << " extensions supported\n";

	glm::mat4 matrix;
	glm::vec4 vec;
	auto test = matrix * vec;

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();


	Engine engine;

	engine.Init();

	while (engine.IsUpdate())
	{
		Timer* timer = Timer::GetTimer();
		float dt = static_cast<float>(timer->GetDeltaTime());
		int frame = timer->GetFPSFrame();
		if (frame >= 0)
		{
			// Print frame
		}
		Timer::GetTimer()->Reset();
		engine.Update(dt);
	}

	engine.Clean();

	return 0;
}