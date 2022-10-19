/******************************************************************************
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   Window.h
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 06.08.2021
	Header file for window using GLEW
******************************************************************************/
#pragma once
#include <string>
#include <glm/vec2.hpp>

struct GLFWwindow;
struct GLFWmonitor;
class MyVulkan;

class Window
{
public:
	friend MyVulkan;
public:
	Window()
		: glfwWindow(nullptr), windowFramebufferResized(false), isPathDropped(false), path(nullptr)
	{}

	bool CreateWindowGLFW(const int& width, const int& height, const char* title, GLFWmonitor* monitor, GLFWwindow* share);
	bool ShouldWindowClose();
	void PollWindowEvents();
	void CloseWindow();

	void ShutWindowDown();

	void SetWindowTitle(const std::string& newTitle);

	void SetWindowFramebufferResized(bool resized);
	bool GetWindowFramebuffer();

	void SetDroppedPath(const char* path);
	bool IsPathDropped();

	void DisplayMessage(std::string title, std::string message);

	glm::ivec2 GetWindowSize();
	void SetWindowSize(int width, int height);
private:
	GLFWwindow* glfwWindow;

	bool windowFramebufferResized;

	bool isPathDropped;
	char* path;

	glm::ivec2 windowSize;
};