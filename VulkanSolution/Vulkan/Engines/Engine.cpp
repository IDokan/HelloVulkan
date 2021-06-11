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

bool Engine::Init()
{
	// Vulkan Init
	


	//A generic application info structure
	VkApplicationInfo applicationInfo{};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pNext = nullptr;
	applicationInfo.pApplicationName = "Sinil's Hello Vulkan";
	applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.pEngineName = "Sinil Engine";
	applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.apiVersion = VK_API_VERSION_1_0;
	// application.Info.apiVersion = ; it contains the version of Vulkan API that my application is expecting to run on. This should be set to the absolute minimum version

	// Create the instance
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
		return false;
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

	if (const VkResult physicalDeviceResult = vkEnumeratePhysicalDevices(instance, &numOfPhysicalDevices, arrayOfPhysicalDevices.get());
		physicalDeviceResult != VK_SUCCESS)
	{
		std::cout << "The second Procedure with physical devices is failed! (Logically, should not be failed)" << std::endl;
		return false;
	}


	//TODO: bellow code does not use second physical device even available. Should change the code.
	VkPhysicalDeviceProperties property{};
	vkGetPhysicalDeviceProperties(arrayOfPhysicalDevices[0], &property);

	VkPhysicalDeviceMemoryProperties memoryProperties;
	// Get the memory properties of the physical device.
	vkGetPhysicalDeviceMemoryProperties(arrayOfPhysicalDevices[0], &memoryProperties);
	memoryProperties.memoryTypes[0].propertyFlags;
	memoryProperties.memoryHeaps[memoryProperties.memoryTypes[0].heapIndex];

	// First determine the number of queue families supported by the physical device.
	
	// Device queues
	uint32_t numOfQueueFamilyProperty = 99;
	std::shared_ptr<VkQueueFamilyProperties[]> familyProperties = nullptr;
	// Bellow code works similar with vkEnumeratePhysicalDevice()
	vkGetPhysicalDeviceQueueFamilyProperties(arrayOfPhysicalDevices[0], &numOfQueueFamilyProperty, familyProperties.get());
	// Allocate enough space for the queue property structures.
	familyProperties = std::make_shared<VkQueueFamilyProperties[]>(numOfQueueFamilyProperty);
	// Now query the actual properties of the queue families.
	vkGetPhysicalDeviceQueueFamilyProperties(arrayOfPhysicalDevices[0], &numOfQueueFamilyProperty, familyProperties.get());

	// Create Logical Device
	VkDeviceQueueCreateInfo logicalDeviceQueueCreateInfo{};
	logicalDeviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	logicalDeviceQueueCreateInfo.pNext = nullptr;
	logicalDeviceQueueCreateInfo.flags = 0;
	logicalDeviceQueueCreateInfo.queueFamilyIndex = 0;
	logicalDeviceQueueCreateInfo.queueCount = 1;
	logicalDeviceQueueCreateInfo.pQueuePriorities = nullptr;

	VkPhysicalDeviceFeatures supportedFeatures{};
	vkGetPhysicalDeviceFeatures(arrayOfPhysicalDevices[0], &supportedFeatures);
	VkPhysicalDeviceFeatures requiredFeatures{};
	requiredFeatures.multiDrawIndirect = supportedFeatures.multiDrawIndirect;
	requiredFeatures.tessellationShader = VK_TRUE;
	requiredFeatures.geometryShader = VK_TRUE;

	VkDeviceCreateInfo logicalDeviceCreateInfo{};
	logicalDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	logicalDeviceCreateInfo.pNext = 0;
	logicalDeviceCreateInfo.flags = 0;
	logicalDeviceCreateInfo.queueCreateInfoCount = 1;
	logicalDeviceCreateInfo.pQueueCreateInfos = &logicalDeviceQueueCreateInfo;
	// We are going to cover layer and extension later in this chapter.
	logicalDeviceCreateInfo.enabledLayerCount = 0;
	logicalDeviceCreateInfo.ppEnabledLayerNames = nullptr;
	logicalDeviceCreateInfo.enabledExtensionCount = 0;
	logicalDeviceCreateInfo.ppEnabledExtensionNames = nullptr;
	logicalDeviceCreateInfo.pEnabledFeatures = &requiredFeatures;

	VkDevice logicalDevice{};
	if (VkResult logicalDeviceCreation = vkCreateDevice(arrayOfPhysicalDevices[0], &logicalDeviceCreateInfo, useInternalAllocator, &logicalDevice);
		logicalDeviceCreation != VK_SUCCESS)
	{
		std::cout << "Vulkan logical device creation is failed!" << std::endl;
		return false;
	}

	std::cout << "Num of queue family property is : " << numOfPhysicalDevices << std::endl;

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
}

bool Engine::IsUpdate()
{
	return isUpdate;
}
