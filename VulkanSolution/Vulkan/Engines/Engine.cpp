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
#include "Vulkan/vulkan.hpp"

Engine::Engine()
	: isUpdate(true), window(new Window())
{
}

Engine::~Engine()
{
	delete window;
}

void Engine::Init()
{
	// Vulkan Init
	VkApplicationInfo applicationInfo{};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pNext = nullptr;
	applicationInfo.pApplicationName = "Sinil's Hello Vulkan";
	applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.pEngineName = "Sinil Engine";
	applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.apiVersion = VK_API_VERSION_1_0;
	// application.Info.apiVersion = ; it contains the version of Vulkan API that my application is expecting to run on. This should be set to the absolute minimum version

	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = nullptr;
	instanceCreateInfo.flags = 0;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;
	
	
	VkInstance instance{};
	VkAllocationCallbacks* useInternalAllocator = nullptr;
	
	if (VkResult instanceResult = vkCreateInstance(&instanceCreateInfo, useInternalAllocator, &instance); 
		instanceResult != VK_SUCCESS)
	{
		std::cout << "Could not create instance" << std::endl;
	}

	uint32_t numOfPhysicalDevices = 99;
	std::shared_ptr<VkPhysicalDevice[]> arrayOfPhysicalDevices = nullptr;
	// second parameter of VK physical devices function works both input & output.
	// As an output, the parameter get how many physical devices I can use.
	// As an input, The maximum number of devices I can control in this application.
	// Once we want to know how many devices available in the system, give final parameter nullptr.(second parameter still should be valid pointer)
	// Then call the same function again with the final parameter set to an array that has been appropriately sized for the number what we have known.
	if (const VkResult physicalDeviceResult = vkEnumeratePhysicalDevices(instance, &numOfPhysicalDevices, arrayOfPhysicalDevices.get()); 
		physicalDeviceResult != VK_SUCCESS)
	{
		std::cout << "The first Procedure with physical devices is failed! (Originally, it might be failed)" << std::endl;
	}
	arrayOfPhysicalDevices = std::make_shared<VkPhysicalDevice[]>(numOfPhysicalDevices);

	if (const VkResult physicalDeviceResult = vkEnumeratePhysicalDevices(instance, & numOfPhysicalDevices, arrayOfPhysicalDevices.get());
		physicalDeviceResult != VK_SUCCESS)
	{
		std::cout << "The second Procedure with physical devices is failed! (Logically, should not be failed)" << std::endl;
	}


	std::cout << "How many physical devices I can control in this device : " << numOfPhysicalDevices << std::endl;


	window->CreateWindow(800, 600, "Vulkan Window", nullptr, nullptr);

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
}

bool Engine::IsUpdate()
{
	return isUpdate;
}
