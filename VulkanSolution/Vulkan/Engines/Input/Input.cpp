/* Start Header -------------------------------------------------------
Copyright (C) FALL2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Input.cpp
Purpose: Class which manage all inputs
Language: C++
Platform: Windows SDK version: 10.0.19041.0, OS: Windows 10. GPU: NVIDIA GeForce840M. OpenGL Driver version: 10.18.15.4279, 8-24-2015
Project: sinil.kang_CS300_1
Author: Sinil Kang = sinil.kang = Colleague ID: 0052782
Creation date: 9/25/2021
End Header --------------------------------------------------------*/

#include "Input.h"
#include <chrono>

Input input;

Input::Input()
{
	xOffset = 0.0;
	yOffset = 0.0;

	keyPressed.reset();
	keyReleased.set();
	keyTriggered.reset();
}

void Input::Update(float dt)
{
	input.TriggeredReset();
	input.SetPresentMousePosition(input.GetMousePosition());
}

void Input::TriggeredReset()
{
	keyTriggered.reset();
	mouseButtonDoubleClicked.reset();
	mouseButtonTriggered.reset();

	xOffset = 0;
	yOffset = 0;
}

void Input::SetKeyboardInput(int key, int action)
{
	if (key >= GLFW_KEY_LAST || key == GLFW_KEY_UNKNOWN)
	{
		return;
	}

	switch (action)
	{
	case GLFW_PRESS:
	{
		keyPressed.set(key);
		keyTriggered.set(key);
		keyReleased.reset(key);
		break;
	}
	case GLFW_RELEASE:
	{
		keyPressed.reset(key);
		keyTriggered.reset(key);
		keyReleased.set(key);
		break;
	}
	default:
		break;
	}
}

void Input::SetMouseButtonInput(int button, int action)
{
	switch (action)
	{
	case GLFW_PRESS:
		mouseButtonPressed.set(button);
		mouseButtonReleased.reset(button);
		mouseButtonTriggered.set(button);
		break;
	case GLFW_RELEASE:
	{
		static auto before = std::chrono::system_clock::now();
		auto now = std::chrono::system_clock::now();
		double diffMs = std::chrono::duration<double, std::milli>(now - before).count();
		before = now;
		if (diffMs > 10 && diffMs < 200)
		{
			mouseButtonDoubleClicked.set(button);
		}
		else
		{
			mouseButtonDoubleClicked.reset(button);
		}
		mouseButtonPressed.reset(button);
		mouseButtonReleased.set(button);
		mouseButtonTriggered.reset(button);
		break;
	}
	default:
		break;
	}
}

void Input::SetMouseWheel(double x, double y)
{
	xOffset = x;
	yOffset = y;
}

bool Input::IsKeyTriggered(int key)
{
	return keyTriggered[key];
}

bool Input::IsKeyPressed(int key)
{
	return keyPressed[key];
}

bool Input::IsKeyReleased(int key)
{
	return keyReleased[key];
}

glm::ivec2 Input::GetPresentMousePosition() const noexcept
{
	return presentMousePosition;
}

void Input::SetPresentMousePosition(const glm::ivec2& mousePosition_) noexcept
{
	presentMousePosition = mousePosition_;
}

glm::ivec2 Input::GetMousePosition() const noexcept
{
	return mousePosition;
}

bool Input::IsMouseButtonTriggered(int button)
{
	return mouseButtonTriggered[button];
}

bool Input::IsMouseButtonPressed(int button)
{
	return mouseButtonPressed[button];
}

bool Input::IsMouseButtonReleased(int button)
{
	return mouseButtonReleased[button];
}

bool Input::IsMouseDoubleClicked(int button)
{
	return mouseButtonDoubleClicked[button];
}

double Input::MouseWheelScroll()
{
	return yOffset;
}

void Input::SetMousePos(GLFWwindow* window, int xPos, int yPos)
{
	SetPresentMousePosition(mousePosition);
	mousePosition.x = xPos;
	mousePosition.y = yPos;
}