/******************************************************************************
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   Window.cpp
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 06.08.2021
	Source file for window using GLEW
******************************************************************************/
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Window.h"

bool Window::CreateWindow(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share)
{
	if (window != nullptr)
	{
		ShutWindowDown();
	}

	if (!glfwInit())
	{
		return false;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window = glfwCreateWindow(width, height, title, monitor, share);
	return true;
}

bool Window::ShouldWindowClose()
{
	if (window == nullptr)
	{
		return true;
	}

	return glfwWindowShouldClose(window);
}

void Window::PollWindowEvents()
{
	glfwPollEvents();
}

void Window::ShutWindowDown()
{
	if (window == nullptr)
	{
		return;
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	window = nullptr;
}
