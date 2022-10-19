/******************************************************************************
Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   Input.h
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 10.18.2022 (Birthday of Suhwan)
	header file for input system.
******************************************************************************/
#pragma once
#include <bitset>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Input
{
public:
	Input();
	void Update(float dt);
	void TriggeredReset();
	void SetKeyboardInput(int key, int action);
	void SetMousePos(GLFWwindow* window, int xPos, int yPos);
	void SetMouseButtonInput(int button, int action);
	void SetMouseWheel(double x, double y);

	bool IsKeyTriggered(int key);
	bool IsKeyPressed(int key);
	bool IsKeyReleased(int key);

	glm::ivec2 GetPresentMousePosition() const noexcept;
	void SetPresentMousePosition(const glm::ivec2& mousePosition) noexcept;
	glm::ivec2 GetMousePosition() const  noexcept;
	double MouseWheelScroll();
	bool IsMouseButtonPressed(int button);
	bool IsMouseButtonTriggered(int button);
	bool IsMouseButtonReleased(int button);
	bool IsMouseDoubleClicked(int button);

private:
	std::bitset<GLFW_KEY_LAST> keyTriggered;
	std::bitset<GLFW_KEY_LAST> keyPressed;
	std::bitset<GLFW_KEY_LAST> keyReleased;

	glm::ivec2 presentMousePosition{};
	glm::ivec2 mousePosition{};
	std::bitset<GLFW_MOUSE_BUTTON_LAST> mouseButtonTriggered;
	std::bitset<GLFW_MOUSE_BUTTON_LAST> mouseButtonPressed;
	std::bitset<GLFW_MOUSE_BUTTON_LAST> mouseButtonReleased;
	std::bitset<GLFW_MOUSE_BUTTON_LAST> mouseButtonDoubleClicked;

	double xOffset = 0.0;
	double yOffset = 0.0;
};

extern Input input;