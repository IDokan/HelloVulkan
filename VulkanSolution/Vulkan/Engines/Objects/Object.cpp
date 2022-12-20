/******************************************************************************
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   Object.cpp
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 12.19.2022
	Source file for object.
******************************************************************************/
#include <../Engines/Objects/Object.h>

static int globalID = 0;

Object::Object(std::string name)
	:name(name), id(globalID++)
{
}

Object::~Object()
{
}
