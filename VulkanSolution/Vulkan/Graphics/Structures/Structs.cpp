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

Skeleton::~Skeleton()
{
}

void Skeleton::Update(float dt)
{
	// Iterate bones but usually useful bones are at the tails.
	// It might be bad.
		// Probabily use reversed iterator for better performance.
	for (Bone* bone : bones)
	{
		bone->Update(dt);
	}
}

void Skeleton::AddBone(std::string name, int parentID)
{
	Bone* newBone = new Bone(name, parentID, boneSize);
	bones.push_back(newBone);
	boneSize += 1;
}

void Skeleton::AddBone(Bone* newBone)
{
	bones.push_back(newBone);
	boneSize += 1;
}

int Skeleton::GetBoneIDByName(const std::string& name)
{
	int id = 0;
	for (Bone* bone : bones)
	{
		if (bone->name.compare(name) == 0)
		{
			return id;
		}
		id++;
	}
	// If there is no name in skeleton,
	return INT32_MIN;
}

const Bone* Skeleton::GetBoneByBoneID(int boneID)
{
	return bones[boneID];
}

const Bone* Skeleton::GetBoneByName(const std::string& name)
{
	for (const Bone* bone : bones)
	{
		if (bone->name.compare(name) == 0)
		{
			return bone;
		}
	}

	return bones.front();
}

Bone& Skeleton::GetBoneReferenceByName(const std::string& name)
{
	return const_cast<Bone&>(*GetBoneByName(name));
}

std::string Skeleton::GetBoneNameByID(unsigned int boneID)
{
	return bones[boneID]->name;
}

size_t Skeleton::GetSkeletonSize()
{
	return boneSize;
}

void Skeleton::Clear()
{
	for (Bone* bone : bones)
	{
		delete bone;
	}
	bones.clear();
	boneSize = 0;
}

void Skeleton::GetToBoneFromUnit(std::vector<glm::mat4>& data)
{
	data.resize(boneSize);
	for (int i = 0; i < boneSize; i++)
	{
		data[i] = bones[i]->toBoneFromUnit;
	}
}

void Skeleton::GetToModelFromBone(std::vector<glm::mat4>& data)
{
	data.resize(boneSize);
	for (int i = 0; i < boneSize; i++)
	{
		data[i] = bones[i]->toModelFromBone;
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

void Bone::Update(float dt)
{
	return;
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

Bone::~Bone()
{
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

bool operator<(const glm::vec3& lhs, const glm::vec3& rhs)
{
	if (lhs.x < rhs.x)
	{
		return true;
	}
	else if (lhs.y < rhs.y)
	{
		return true;
	}
	else if (lhs.z < rhs.z)
	{
		return true;
	}
	return false;
}

bool operator>(const glm::vec3& lhs, const glm::vec3& rhs)
{
	if (lhs.x > rhs.x)
	{
		return true;
	}
	else if (lhs.y > rhs.y)
	{
		return true;
	}
	else if (lhs.z > rhs.z)
	{
		return true;
	}
	return false;
}

bool operator<(const glm::vec2& lhs, const glm::vec2& rhs)
{
	if (lhs.x < rhs.x)
	{
		return true;
	}
	else if (lhs.y < rhs.y)
	{
		return true;
	}
	return false;
}

bool operator>(const glm::vec2& lhs, const glm::vec2& rhs)
{
	if (lhs.x > rhs.x)
	{
		return true;
	}
	else if (lhs.y > rhs.y)
	{
		return true;
	}
	return false;
}

bool operator<(glm::vec3&& lhs, glm::vec3&& rhs)
{
	if (lhs.x < rhs.x)
	{
		return true;
	}
	else if (lhs.y < rhs.y)
	{
		return true;
	}
	else if (lhs.z < rhs.z)
	{
		return true;
	}
	return false;
}

bool operator>(glm::vec3&& lhs, glm::vec3&& rhs)
{
	if (lhs.x > rhs.x)
	{
		return true;
	}
	else if (lhs.y > rhs.y)
	{
		return true;
	}
	else if (lhs.z > rhs.z)
	{
		return true;
	}
	return false;
}

bool operator<(glm::vec2&& lhs, glm::vec2&& rhs)
{
	if (lhs.x < rhs.x)
	{
		return true;
	}
	else if (lhs.y < rhs.y)
	{
		return true;
	}
	return false;
}

bool operator>(glm::vec2&& lhs, glm::vec2&& rhs)
{
	if (lhs.x > rhs.x)
	{
		return true;
	}
	else if (lhs.y > rhs.y)
	{
		return true;
	}
	return false;
}

bool vec3Compare::operator()(const glm::vec3& lhs, const glm::vec3& rhs) const
{
	//if (lhs.x < rhs.x)
	//{
	//	return true;
	//}
	//else if (lhs.y < rhs.y)
	//{
	//	return true;
	//}
	//else if (lhs.z < rhs.z)
	//{
	//	return true;
	//}
	return lhs.x < rhs.x;
}

bool vec2Compare::operator()(const glm::vec2& lhs, const glm::vec2& rhs) const
{
	//if (lhs.x < rhs.x)
	//{
	//	return true;
	//}
	//else if (lhs.y < rhs.y)
	//{
	//	return true;
	//}
	return lhs.x < rhs.x;
}

JiggleBone::JiggleBone()
	: Bone(), isUpdateJigglePhysics(false), modelUnitTranslation(glm::vec3(0.f)), customPhysicsMatrix(glm::mat4(0.f)), physics()
{
}

JiggleBone::JiggleBone(std::string name, int parentID, int id, glm::mat4 toBoneFromModel, glm::mat4 toModelFromBone, glm::vec3 modelUnitTranslation)
	: Bone(name, parentID, id, toBoneFromModel, toModelFromBone), isUpdateJigglePhysics(false), modelUnitTranslation(modelUnitTranslation), customPhysicsMatrix(glm::identity<glm::mat4>()), physics()
{
}

JiggleBone::JiggleBone(const JiggleBone& jb)
	: Bone(jb), isUpdateJigglePhysics(jb.isUpdateJigglePhysics), modelUnitTranslation(jb.modelUnitTranslation), customPhysicsMatrix(jb.customPhysicsMatrix), physics(jb.physics)
{
}

JiggleBone::JiggleBone(JiggleBone&& jb)
	: Bone(jb), isUpdateJigglePhysics(jb.isUpdateJigglePhysics), modelUnitTranslation(jb.modelUnitTranslation), customPhysicsMatrix(jb.customPhysicsMatrix), physics(jb.physics)
{
	
}

void JiggleBone::Update(float dt)
{
	if (isUpdateJigglePhysics == false)
	{
		return;
	}

	constexpr glm::vec3 force = glm::vec3(0.f, -4.9f, 0.f);

	physics.UpdateByForce(dt, force);

	//// @@ TODO: Implement jiggle physics here
	//toModelFromBone = 
	//	glm::translate(modelUnitTranslation) * 
	//	glm::rotate(glm::half_pi<float>() / 3.f * dt, glm::vec3(1.f, 0.f, 0.f)) * 
	//	glm::translate(-modelUnitTranslation) * 
	//	toModelFromBone;

	customPhysicsMatrix = glm::translate(physics.GetCenterOfMass());
}

JiggleBone& JiggleBone::operator=(const JiggleBone& jb)
{
	name = jb.name;
	parentID = jb.parentID;
	id = jb.id;
	toBoneFromUnit = jb.toBoneFromUnit;
	toModelFromBone = jb.toModelFromBone;
	isUpdateJigglePhysics = jb.isUpdateJigglePhysics;
	modelUnitTranslation = jb.modelUnitTranslation;
	customPhysicsMatrix = jb.customPhysicsMatrix;
	physics = jb.physics;
	return *this;
}

JiggleBone& JiggleBone::operator=(JiggleBone&& jb)
{
	name = jb.name;
	parentID = jb.parentID;
	id = jb.id;
	toBoneFromUnit = jb.toBoneFromUnit;
	toModelFromBone = jb.toModelFromBone;
	isUpdateJigglePhysics = jb.isUpdateJigglePhysics;
	modelUnitTranslation = jb.modelUnitTranslation;
	customPhysicsMatrix = jb.customPhysicsMatrix;
	physics = jb.physics;
	return *this;
}

JiggleBone::~JiggleBone()
{
}

void JiggleBone::SetIsUpdateJigglePhysics(bool isUpdate)
{
	isUpdateJigglePhysics = isUpdate;

	if (isUpdateJigglePhysics)
	{
		physics.Initialize();
	}
}

Physics::Physics()
	:centerOfMass(), translation(), linearMomentum(), linearVelocity(), totalMass(), force()
{
}

Physics::Physics(const Physics& p)
	:centerOfMass(p.centerOfMass), translation(p.translation), linearMomentum(p.linearMomentum), linearVelocity(p.linearVelocity), totalMass(p.totalMass), force(p.force)
{
}

Physics::Physics(Physics&& p)
	: centerOfMass(p.centerOfMass), translation(p.translation), linearMomentum(p.linearMomentum), linearVelocity(p.linearVelocity), totalMass(p.totalMass), force(p.force)
{
}

Physics& Physics::operator=(const Physics& p)
{
	centerOfMass = p.centerOfMass;
	translation = p.translation;
	linearMomentum = p.linearMomentum;
	linearVelocity = p.linearVelocity;
	totalMass = p.totalMass;
	force = p.force;
	return *this;
}

Physics& Physics::operator=(Physics&& p)
{
	centerOfMass = p.centerOfMass;
	translation = p.translation;
	linearMomentum = p.linearMomentum;
	linearVelocity = p.linearVelocity;
	totalMass = p.totalMass;
	force = p.force;
	return *this;
}

Physics::~Physics()
{
}

void Physics::Initialize()
{
	centerOfMass = glm::vec3(0.f, 0.f, 0.f);

	// @@ TODO: implement details of total mass later.
	totalMass = 1.f;
}

void Physics::UpdateByForce(float dt, glm::vec3 _force)
{
	force = _force;
	linearMomentum = dt * force;
	linearVelocity = linearMomentum / totalMass;
	translation += dt * linearVelocity;
	centerOfMass = centerOfMass + translation;
}

glm::vec3 Physics::GetCenterOfMass()
{
	return centerOfMass;
}
