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
#include <string>
#include <iostream>

#include "Engine.h"
#include "Engines/Window.h"
#include "Timer.h"
#include "Graphics/MyVulkan.h"
#include "vulkan/vulkan_core.h"

Engine::Engine()
	: isUpdate(true), window(new Window())
{
}

Engine::~Engine()
{
	delete window;
}

bool Engine::Init()
{
	// Vulkan Init
	MyVulkan::InitVulkan("Sinil's Hello Vulkan", VK_MAKE_VERSION(1, 0, 0));

	window->CreateWindow(800, 600, "Vulkan Window", nullptr, nullptr);

	

	return true;
}

void Engine::Update()
{
	// Calculate dt, and FPS to show up on window's title bar
	Timer* timer = Timer::GetTimer();
	float dt = static_cast<float>(timer->GetDeltaTime());
	(dt);
	int frame = timer->GetFPSFrame();
	if (frame >= 0)
	{
		window->SetWindowTitle(std::string("Vulkan Window, FPS : " + frame));
	}
	Timer::GetTimer()->Reset();

	// Update window when it is alive.
	while (window->ShouldWindowClose() == false)
	{
		window->PollWindowEvents();
	}
}

void Engine::Clean()
{
	window->ShutWindowDown();

	MyVulkan::CleanVulkan();
}

bool Engine::IsUpdate()
{
	return isUpdate && (!window->ShouldWindowClose());
}
