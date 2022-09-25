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
#include <string>
#include "Window.h"

namespace CallbackFuncs
{
	void onError(int error, const char* description)
	{
		printf("GLFW Error %d: %s\n", error, description);
	}

	void WindowClose(GLFWwindow* window)
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}

	void FramebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		app->SetWindowFramebufferResized(true);
	}
}

bool Window::CreateWindow(const int& width, const int& height, const char* title, GLFWmonitor* monitor, GLFWwindow* share)
{
	if (glfwWindow != nullptr)
	{
		ShutWindowDown();
	}

	if (!glfwInit())
	{
		return false;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwSetErrorCallback(CallbackFuncs::onError);

	glfwWindow = glfwCreateWindow(width, height, title, monitor, share);

	glfwSetWindowUserPointer(glfwWindow, this);
	glfwSetWindowCloseCallback(glfwWindow, CallbackFuncs::WindowClose);
	glfwSetFramebufferSizeCallback(glfwWindow, CallbackFuncs::FramebufferResizeCallback);

	if (!glfwVulkanSupported())
	{
		printf("GLFW: Vulkan Not Supported\n");
		return false;
	}

	return true;
}

bool Window::ShouldWindowClose()
{
	if (glfwWindow == nullptr)
	{
		return true;
	}

	return glfwWindowShouldClose(glfwWindow);
}

void Window::PollWindowEvents()
{
	glfwPollEvents();
}

void Window::CloseWindow()
{
	CallbackFuncs::WindowClose(glfwWindow);
}

void Window::ShutWindowDown()
{
	if (glfwWindow == nullptr)
	{
		return;
	}
	glfwDestroyWindow(glfwWindow);
	glfwTerminate();
	glfwWindow = nullptr;
}

void Window::SetWindowTitle(const std::string& newTitle)
{
	glfwSetWindowTitle(glfwWindow, newTitle.c_str());
}

void Window::SetWindowFramebufferResized(bool resized)
{
	windowFramebufferResized = resized;
}

bool Window::GetWindowFramebuffer()
{
	return windowFramebufferResized;
}
