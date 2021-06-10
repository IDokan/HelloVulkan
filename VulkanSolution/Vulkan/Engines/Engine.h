/******************************************************************************
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   Engine.h
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 06.08.2021
	header file for engine.
******************************************************************************/
#pragma once

class Window;

class Engine
{
public:
	Engine();
	~Engine();

	// return whether initialization is succeed or not.
	bool Init();
	void Update();
	void Clean();

	bool IsUpdate();

private:
	bool isUpdate;
	Window* window;
};