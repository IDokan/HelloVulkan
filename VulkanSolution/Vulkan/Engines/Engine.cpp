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
#include "Graphics/MyScene.h"
#include "Graphics/Graphics.h"
#include "vulkan/vulkan_core.h"
#include "ImGUI/myGUI.h"
#include "Input/Input.h"

Engine::Engine()
	: isUpdate(true), window(new Window()), scene(new MyScene(window)), graphics(new Graphics())
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

	if (graphics->InitVulkan("Sinil's Hello Vulkan", VK_MAKE_VERSION(1, 0, 0), window) == false)
	{
		window->DisplayMessage("Init Failed!", "Init Graphics resources(Vulkan) has Failed!");
		return false;
	}

	if (scene->InitScene(graphics) == false)
	{
		window->DisplayMessage("Init Failed!", "Init Scene information has Failed!");
		return false;
	}

	scene->InitGUI();

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
		scene->LoadNewModel();
	}

	MyImGUI::DrawGUI();
	
	bool canDrawing = graphics->StartDrawing();
	if (canDrawing)
	{
		scene->DrawFrame(dt, graphics->GetCommandBuffer(), graphics->GetCurrentFrameID());
		graphics->EndDrawing();
	}
}

void Engine::Clean()
{
	window->ShutWindowDown();

	MyImGUI::DestroyGUIResources();

	scene->CleanScene();

	graphics->CleanVulkan();
}

bool Engine::IsUpdate()
{
	return isUpdate && (!window->ShouldWindowClose());
}
