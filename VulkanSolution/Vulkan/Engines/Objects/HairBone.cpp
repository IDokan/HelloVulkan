#include "HairBone.h"

HairBone::HairBone(std::string name, glm::vec3 initPosition)
	:Object(name), size(1), bones(90, glm::vec4(initPosition.x, initPosition.y, initPosition.z, 1.f))
{
}

HairBone::~HairBone()
{
}

bool HairBone::Init()
{
	return true;
}

void HairBone::Update(float dt)
{
}

void HairBone::Clean()
{
}

size_t HairBone::GetHairBoneMaxDataSize()
{
	return sizeof(glm::vec4) * 90;
}

size_t HairBone::GetBoneSize()
{
	return size;
}

void HairBone::AddBone()
{
	if (size >= 90)
	{
		return;
	}
	bones[size] = bones[size - 1];
	++size;

}

void HairBone::RemoveBone()
{
	if (size <= 1)
	{
		return;
	}
	--size;
}


void* HairBone::GetBoneData()
{
	return bones.data();
}
