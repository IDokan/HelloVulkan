/******************************************************************************
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   main.cpp
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 06.08.2021
	main function.
******************************************************************************/
#include "GLMath.h"

#include <iostream>

#include "Engines/Engine.h"
#include "Engines/Timer.h"

int main()
{
	Engine* engine = new Engine();

	if (engine->Init() == false)
	{
		std::cout << "Initialization Failed!" << std::endl;
		return 0;
	}

	while (engine->IsUpdate())
	{
		engine->Update();
	}

	engine->Clean();

	return 0;
}