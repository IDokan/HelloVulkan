#include "Engine.h"
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

#include "Engine.h"
#include "Engines/Window.h"

Engine::Engine()
	: isUpdate(true)
{
}

Engine::~Engine()
{
}

void Engine::Init()
{
}

void Engine::Update(float /*dt*/)
{
	//while (window->)
	{

	}
}

void Engine::Clean()
{
}

bool Engine::IsUpdate()
{
	return isUpdate;
}
