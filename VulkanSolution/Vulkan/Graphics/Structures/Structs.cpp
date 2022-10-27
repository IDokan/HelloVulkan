#include "Structs.h"

Mesh::Mesh()
{
}

Mesh::Mesh(const std::string& name, const std::vector<uint32_t>& indices, const std::vector<Vertex>& vertices)
	: meshName(name), indices(indices), vertices(vertices)
{
}

Mesh::Mesh(const Mesh& m)
	:meshName(m.meshName), indices(m.indices), vertices(m.vertices)
{
}

Mesh::Mesh(Mesh&& m)
	: meshName(m.meshName), indices(m.indices), vertices(m.vertices)
{
}

Mesh& Mesh::operator=(const Mesh& m)
{
	meshName = m.meshName;
	indices = m.indices;
	vertices = m.vertices;
	return *this;
}

Mesh& Mesh::operator=(Mesh&& m)
{
	meshName = m.meshName;
	indices = m.indices;
	vertices = m.vertices;
	return *this;
}

void Skeleton::AddBone(std::string name, int parentID)
{
	bones.push_back(Bone(name, parentID));
}

int Skeleton::FindBoneIDByName(const std::string& name)
{
	int id = 0;
	for (const Bone& bone : bones)
	{
		if (bone.name.compare(name) == 0)
		{
			return id;
		}
		id++;
	}
	// If there is no name in skeleton,
	return INT32_MIN;
}

const Bone& Skeleton::GetBoneByBoneID(int boneID)
{
	return bones[boneID];
}

size_t Skeleton::GetSkeletonSize()
{
	return bones.size();
}

void Skeleton::Clear()
{
	bones.clear();
}

Animation::Animation()
	:animationName(), duration(-1.f), tracks()
{
}

Animation::Animation(std::string animationName, float duration, size_t trackSize)
	:animationName(animationName), duration(duration)
{
	tracks.resize(trackSize);
}

Animation::Animation(const Animation& m)
	: animationName(m.animationName), duration(m.duration), tracks(m.tracks)
{
}

Animation::Animation(Animation&& m)
	: animationName(m.animationName), duration(m.duration), tracks(m.tracks)
{
}

Animation& Animation::operator=(const Animation& m)
{
	animationName = m.animationName;
	duration = m.duration;
	tracks = m.tracks;

	return *this;
}

Animation& Animation::operator=(Animation&& m)
{
	animationName = m.animationName;
	duration = m.duration;
	tracks = m.tracks;

	return *this;
}
