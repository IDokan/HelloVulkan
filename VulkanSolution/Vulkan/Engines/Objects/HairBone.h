/******************************************************************************
Copyright (C) 2023 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   HairBone.h
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 1.17.2022
	Header file for Hair bone.
******************************************************************************/

#pragma once
#include <Engines/Objects/Object.h>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <vector>

class HairBone : public Object
{
public:
	HairBone(std::string name, glm::vec3 initPosition = glm::vec3(0.f));
	~HairBone();

	bool Init();
	void Update(float dt);
	void Clean();
	
	size_t GetHairBoneMaxDataSize();
	size_t GetBoneSize();
	void AddBone();
	void RemoveBone();
	void* GetBoneData();
	void SetBoneData(int i, glm::vec4 data);
	glm::vec4 GetBoneData(int i);

private:
	size_t size;
	std::vector<glm::vec4> bones;
};