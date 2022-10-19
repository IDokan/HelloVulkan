/******************************************************************************
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   Engine.cpp
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 06.08.2021
	Source file for engine.
	Contains main game loop and control windows.
******************************************************************************/
#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include <iostream>

#include "Engine.h"
#include "Engines/Window.h"
#include "Timer.h"
#include "Graphics/MyVulkan.h"
#include "vulkan/vulkan_core.h"
#include "ImGUI/myGUI.h"
#include "Input/Input.h"

Engine::Engine()
	: isUpdate(true), window(new Window()), VK(new MyVulkan(window))
{
}

Engine::~Engine()
{
	delete window;
}

bool Engine::Init()
{
	// Vulkan Init

	if (window->CreateWindowGLFW(800, 600, "BMLv", nullptr, nullptr) == false)
	{
		window->DisplayMessage("Init Failed!", "Crating Window has Failed!");
		return false;
	}

	if (VK->InitVulkan("Sinil's Hello Vulkan", VK_MAKE_VERSION(1, 0, 0)) == false)
	{
		window->DisplayMessage("Init Failed!", "Init Vulkan has Failed!");
		return false;
	}

	VK->InitGUI();

	return true;
}

void Engine::Update()
{

	// Calculate dt, and FPS to show up on window's title bar
	Timer* timer = Timer::GetTimer();
	float dt = static_cast<float>(timer->GetDeltaTime());
	int frame = timer->GetFPSFrame();
	if (frame >= 0)
	{
		char buf[32];
		_itoa(frame, buf, 10);
		window->SetWindowTitle(std::string("BMLv, FPS : ") + std::string(buf));
	}
	Timer::GetTimer()->Reset();

	// Update window
	input.Update(dt);
	window->PollWindowEvents();

	if (window->IsPathDropped())
	{
		VK->LoadNewModel();
	}

	MyImGUI::DrawGUI();
	
	VK->DrawFrame(dt);
}

void Engine::Clean()
{
	window->ShutWindowDown();

	MyImGUI::DestroyGUIResources();

	VK->CleanVulkan();
}

bool Engine::IsUpdate()
{
	return isUpdate && (!window->ShouldWindowClose());
}
