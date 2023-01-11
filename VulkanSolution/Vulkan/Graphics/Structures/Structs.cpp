#include "Structs.h"

Mesh::Mesh()
	:meshName(), indices(), vertices(), uniqueVertices()
{
}

Mesh::Mesh(const std::string& name, const std::vector<uint32_t>& indices, const std::vector<Vertex>& vertices, const std::vector<Vertex>& uniqueVertices)
	: meshName(name), indices(indices), vertices(vertices), uniqueVertices(uniqueVertices)
{
}

Mesh::Mesh(const Mesh& m)
	: meshName(m.meshName), indices(m.indices), vertices(m.vertices), uniqueVertices(m.uniqueVertices)
{
}

Mesh::Mesh(Mesh&& m)
	: meshName(m.meshName), indices(m.indices), vertices(m.vertices), uniqueVertices(m.uniqueVertices)
{
}

Mesh& Mesh::operator=(const Mesh& m)
{
	meshName = m.meshName;
	indices = m.indices;
	vertices = m.vertices;
	uniqueVertices = m.uniqueVertices;
	return *this;
}

Mesh& Mesh::operator=(Mesh&& m)
{
	meshName = m.meshName;
	indices = m.indices;
	vertices = m.vertices;
	uniqueVertices = m.uniqueVertices;
	return *this;
}

Skeleton::Skeleton()
	:bones(), boneSize(0)
{
}

void Skeleton::AddBone(std::string name, int parentID)
{
	bones.push_back(Bone(name, parentID, boneSize));
	boneSize += 1;
}

int Skeleton::GetBoneIDByName(const std::string& name)
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

const Bone& Skeleton::GetBoneByName(const std::string& name)
{
	for (const Bone& bone : bones)
	{
		if (bone.name.compare(name) == 0)
		{
			return bone;
		}
	}

	return bones.front();
}

Bone& Skeleton::GetBoneReferenceByName(const std::string& name)
{
	return const_cast<Bone&>(GetBoneByName(name));
}

std::string Skeleton::GetBoneNameByID(unsigned int boneID)
{
	return bones[boneID].name;
}

size_t Skeleton::GetSkeletonSize()
{
	return boneSize;
}

void Skeleton::Clear()
{
	bones.clear();
	boneSize = 0;
}

void Skeleton::GetToBoneFromUnit(std::vector<glm::mat4>& data)
{
	data.resize(boneSize);
	for (int i = 0; i < boneSize; i++)
	{
		data[i] = bones[i].toBoneFromUnit;
	}
}

void Skeleton::GetToModelFromBone(std::vector<glm::mat4>& data)
{
	data.resize(boneSize);
	for (int i = 0; i < boneSize; i++)
	{
		data[i] = bones[i].toModelFromBone;
	}
}

Animation::Animation()
	:animationName(), duration(-1.f), tracks()
{
}

Animation::Animation(std::string animationName, float duration, size_t trackSize)
	: animationName(animationName), duration(duration)
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

Bone::Bone()
	: name(), parentID(-1), id(-1), toBoneFromUnit(), toModelFromBone()
{
}

Bone::Bone(std::string _name, int _parentID, int _id, glm::mat4 _toBoneFromModel, glm::mat4 _toModelFromBone)
	: name(_name), parentID(_parentID), id(_id), toBoneFromUnit(_toBoneFromModel), toModelFromBone(_toModelFromBone)
{

}

Bone::Bone(const Bone& b)
	: name(b.name), parentID(b.parentID), id(b.id), toBoneFromUnit(b.toBoneFromUnit), toModelFromBone(b.toModelFromBone)
{
}

Bone::Bone(Bone&& b)
	: name(b.name), parentID(b.parentID), id(b.id), toBoneFromUnit(b.toBoneFromUnit), toModelFromBone(b.toModelFromBone)
{
}

Bone& Bone::operator=(const Bone& b)
{
	name = b.name;
	parentID = b.parentID;
	id = b.id;
	toBoneFromUnit = b.toBoneFromUnit;
	toModelFromBone = b.toModelFromBone;
	return *this;
}

Bone& Bone::operator=(Bone&& b)
{
	name = b.name;
	parentID = b.parentID;
	id = b.id;
	toBoneFromUnit = b.toBoneFromUnit;
	toModelFromBone = b.toModelFromBone;
	return *this;
}

std::ostream& operator<<(std::ostream& os, const glm::vec4& data)
{
	os << data.x << ", " << data.y << ", " << data.z << ", " << data.w;
	return os;
}

std::ostream& operator<<(std::ostream& os, const glm::vec3& data)
{
	os << data.x << ", " << data.y << ", " << data.z;
	return os;
}

std::ostream& operator<<(std::ostream& os, const glm::vec2& data)
{
	os << data.x << ", " << data.y;
	return os;
}

std::ostream& operator<<(std::ostream& os, const glm::ivec2& data)
{
	os << data.x << ", " << data.y;
	return os;
}