/******************************************************************************
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   Object.h
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 12.19.2022
	header file for object.
******************************************************************************/
#include <string>

class Object
{
public:
	Object(std::string name);
	virtual ~Object();

	virtual bool Init() = 0;
	virtual void Update(float dt) = 0;
	virtual void Clean() = 0;

	std::string GetName();
	unsigned int GetID();

protected:
	std::string name;
	unsigned int id;
};