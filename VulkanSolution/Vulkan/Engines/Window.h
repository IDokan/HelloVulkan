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

struct GLFWwindow;
struct GLFWmonitor;

class Window
{
public:
	Window()
		: window(nullptr)
	{}

	bool CreateWindow(const int& width, const int& height, const char* title, GLFWmonitor* monitor, GLFWwindow* share);
	bool ShouldWindowClose();
	void PollWindowEvents();

	void ShutWindowDown();

	void SetWindowTitle(const std::string& newTitle);

private:
	GLFWwindow* window;
};