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
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#define GLFW_NATIVE_INCLUDE_NONE
#include <GLFW/glfw3native.h>
#include <string>
#include "Window.h"
#include "Input/Input.h"
#include <iostream>

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
		app->SetWindowSize(width, height);
	}

	void DropCallback(GLFWwindow* window, int count, const char** paths)
	{
		auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		for (int i = 0; i < count; i++)
		{
			app->SetDroppedPath(paths[i]);
		}
	}

	void MousePositionCallback(GLFWwindow* window, double xPos, double yPos)
	{
		Window* app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		const glm::ivec2 size = app->GetWindowSize();

		xPos = xPos - (size.x / 2);
		yPos = (size.y / 2) - yPos;

		input.SetMousePos(window, static_cast<int>(xPos), static_cast<int>(yPos));
	}
	void MouseWheelScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
	{
		input.SetMouseWheel(xOffset, yOffset);
	}
	void MouseButtonCallback(GLFWwindow*, int button, int action, int)
	{
		input.SetMouseButtonInput(button, action);
	}
	void KeyCallback(GLFWwindow*, int key, int, int action, int)
	{
		input.SetKeyboardInput(key, action);
	}
}

bool Window::CreateWindowGLFW(const int& width, const int& height, const char* title, GLFWmonitor* monitor, GLFWwindow* share)
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

	SetWindowSize(width, height);
	glfwWindow = glfwCreateWindow(width, height, title, monitor, share);

	glfwSetWindowUserPointer(glfwWindow, this);
	glfwSetWindowCloseCallback(glfwWindow, CallbackFuncs::WindowClose);
	glfwSetFramebufferSizeCallback(glfwWindow, CallbackFuncs::FramebufferResizeCallback);
	glfwSetDropCallback(glfwWindow, CallbackFuncs::DropCallback);

	glfwSetCursorPosCallback(glfwWindow, CallbackFuncs::MousePositionCallback);
	glfwSetScrollCallback(glfwWindow, CallbackFuncs::MouseWheelScrollCallback);
	glfwSetMouseButtonCallback(glfwWindow, CallbackFuncs::MouseButtonCallback);
	glfwSetKeyCallback(glfwWindow, CallbackFuncs::KeyCallback);

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

void Window::SetDroppedPath(const char* _path)
{
	isPathDropped = true;
	if (path != nullptr)
	{
		delete[] path;
	}
	path = new char[strlen(_path)+1];
	strcpy(path, _path);
}

bool Window::IsPathDropped()
{
	return isPathDropped;
}

void Window::DisplayMessage(std::string title, std::string message)
{
	std::wstring titleW = std::wstring(title.begin(), title.end());
	std::wstring messageW = std::wstring(message.begin(), message.end());

	MessageBox(glfwGetWin32Window(glfwWindow), messageW.c_str(), titleW.c_str(), MB_OK | MB_ICONERROR);
}

glm::ivec2 Window::GetWindowSize()
{
	return windowSize;
}

void Window::SetWindowSize(int width, int height)
{
	windowSize.x = width;
	windowSize.y = height;
}
