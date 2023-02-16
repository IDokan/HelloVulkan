#include "Structs.h"

float Physics::GravityScaler = 1.f;
float Physics::SpringScaler = 5.f;
float Physics::DampingScaler = 100.f;
bool Physics::forceApplyFlag = false;

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
	for (auto it = bones.rbegin(); it != bones.rend(); it++)
	{
		Bone* ptr = (*it);

		if (JiggleBone* jbPtr = dynamic_cast<JiggleBone*>(ptr);
			jbPtr == nullptr)
		{
			break;
		}

		ptr->Update(dt);
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
	: name(), parentID(-1), id(-1), toBoneFromUnit(glm::mat4(1.f)), toModelFromBone(glm::mat4(1.f))
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

std::ostream& operator<<(std::ostream& os, const glm::mat3& data)
{
	os << "[ [" << data[0][0] << ", " << data[0][1] << ", " << data[0][2] << "], [" << data[1][0] << ", " << data[1][1] << ", " <<data[1][2] << "], [" << data[2][0] << ", " << data[2][1] << ", " << data[2][2] << "]]";
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
	: Bone(), isUpdateJigglePhysics(false), customPhysicsTranslation(glm::mat4(0.f)), customPhysicsRotation(glm::mat4(0.f)), physics(), parentBonePtr(nullptr)
{
}

JiggleBone::JiggleBone(std::string name, int parentID, int id, glm::mat4 toBoneFromUnit, glm::mat4 toModelFromBone, const Bone* parentBonePtr)
	: Bone(name, parentID, id, toBoneFromUnit, toModelFromBone), isUpdateJigglePhysics(false), customPhysicsTranslation(glm::identity<glm::mat4>()), customPhysicsRotation(glm::identity<glm::mat4>()), physics(), parentBonePtr(parentBonePtr)
{
}

JiggleBone::JiggleBone(const JiggleBone& jb)
	: Bone(jb), isUpdateJigglePhysics(jb.isUpdateJigglePhysics), customPhysicsTranslation(jb.customPhysicsTranslation), customPhysicsRotation(jb.customPhysicsRotation), physics(jb.physics), parentBonePtr(jb.parentBonePtr)
{
}

JiggleBone::JiggleBone(JiggleBone&& jb)
	: Bone(jb), isUpdateJigglePhysics(jb.isUpdateJigglePhysics), customPhysicsTranslation(jb.customPhysicsTranslation), customPhysicsRotation(jb.customPhysicsRotation), physics(jb.physics), parentBonePtr(jb.parentBonePtr)
{
	
}

void JiggleBone::Update(float dt)
{
	if (isUpdateJigglePhysics == false)
	{
		return;
	}

	// @@ Begin of Physics calculation
	glm::vec3 gravityForce= glm::vec3(0.f, -1.f, 0.f) * Physics::GravityScaler;
	glm::vec3 force = glm::vec3(0.f);
	
	glm::vec4 bindPoseDifference = (parentBonePtr->toBoneFromUnit* glm::vec4(0.f, 0.f, 0.f, 1.f));

	glm::vec3 anchorPoint = glm::vec3(bindPoseDifference.x, bindPoseDifference.y, bindPoseDifference.z);
	glm::vec4 exertedAnchorPoint4 = customPhysicsTranslation * glm::translate(anchorPoint) * customPhysicsRotation * glm::translate(-anchorPoint) * parentBonePtr->toBoneFromUnit * glm::vec4(0.f, 0.f, 0.f, 1.f);
	glm::vec3 exertedAnchorPoint = glm::vec3(exertedAnchorPoint4.x, exertedAnchorPoint4.y, exertedAnchorPoint4.z);

	// @@TODO: Implement If parent bone is not anchor?, (In other words, it is also a stick of spring-mass-damper system?)
	// Skip for right now because we are doing making sure it works in simple situation
	//glm::vec3 parentVelocity = glm::vec3(0.f);
	//if (const JiggleBone* parentJb = dynamic_cast<const JiggleBone*>(parentBonePtr);
	//	parentJb != nullptr)
	//{
	//	diffVector -= parentJb->physics.translation;
	//	parentVelocity = parentJb->physics.linearVelocity;
	//}

	// spring force
	glm::vec3 springForce = Physics::SpringScaler * (anchorPoint - exertedAnchorPoint);
	glm::vec3 dampingForce = Physics::DampingScaler * (glm::vec3(0.f) - physics.linearVelocity);

	glm::vec3 finalForce = gravityForce + springForce + dampingForce;

	glm::vec4 exertedPoint4 = customPhysicsTranslation * glm::translate(anchorPoint) * customPhysicsRotation * glm::translate(-anchorPoint) * toBoneFromUnit * glm::vec4(0.f, 0.f, 0.f, 1.f);
	glm::vec3 exertedPoint = glm::vec3(exertedPoint4.x, exertedPoint4.y, exertedPoint4.z);
	glm::vec4 bSideAnchor4 = toBoneFromUnit * glm::vec4(0.f, 0.f, 0.f, 1.f);
	glm::vec3 bSideAnchor = glm::vec3(bSideAnchor4.x, bSideAnchor4.y, bSideAnchor4.z);
	glm::vec3 bSideSpring = bSideAnchor + (glm::vec3(0.f, 0.f, 0.f));
	glm::vec3 bSpringForce = Physics::SpringScaler * (bSideSpring- exertedPoint);
	//finalForce += bSpringForce + dampingForce;

	// the reason why used (anchorPoint - anchorPoint), (x - y), x is the position where exerted on.
	glm::vec3 torquePoint = (exertedPoint - anchorPoint) - physics.centerOfMass;	// bSide
	glm::vec3 torquePoint2 = (exertedAnchorPoint - anchorPoint) - physics.centerOfMass;	// aSide
	glm::vec3 torque = glm::cross(torquePoint, (0.5f * gravityForce) /* + bSpringForce + dampingForce*/);
	glm::vec3 torque2 = glm::cross(torquePoint2, (0.5f * gravityForce) + springForce + dampingForce);
	if (Physics::forceApplyFlag)
	{
		physics.UpdateByForce(dt, finalForce, torque + torque2);
	}
	else
	{
		physics.UpdateByForce(dt, glm::vec3(0.f), glm::vec3(0.f));
	}

	customPhysicsTranslation = glm::translate(physics.translation);
	customPhysicsRotation = glm::mat4(physics.rotation);
}

JiggleBone& JiggleBone::operator=(const JiggleBone& jb)
{
	name = jb.name;
	parentID = jb.parentID;
	id = jb.id;
	toBoneFromUnit = jb.toBoneFromUnit;
	toModelFromBone = jb.toModelFromBone;
	isUpdateJigglePhysics = jb.isUpdateJigglePhysics;
	customPhysicsTranslation = jb.customPhysicsTranslation;
	customPhysicsRotation = jb.customPhysicsRotation;
	physics = jb.physics;
	parentBonePtr = jb.parentBonePtr;
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
	customPhysicsTranslation = jb.customPhysicsTranslation;
	customPhysicsRotation = jb.customPhysicsRotation;
	physics = jb.physics;
	parentBonePtr = jb.parentBonePtr;
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
		customPhysicsTranslation = glm::mat4(1.f);
		customPhysicsRotation = glm::mat4(1.f);
		physics.Initialize(parentBonePtr->toBoneFromUnit);
	}
}

void JiggleBone::AddVertices(const std::vector<glm::vec3>& _vertices)
{
	physics.initVertices = _vertices;
	physics.vertices = _vertices;
}

Physics::Physics()
	:centerOfMass(), initCenterOfMass(), translation(), linearMomentum(), linearVelocity(), force(), rotation(), angularMomentum(), inertiaTensorInverse(), inertiaTensorObj(), torque(), totalMass(), vertices()
{
}

Physics::Physics(const Physics& p)
	:centerOfMass(p.centerOfMass), initCenterOfMass(p.initCenterOfMass), translation(p.translation), linearMomentum(p.linearMomentum), linearVelocity(p.linearVelocity), force(p.force), rotation(p.rotation), angularMomentum(p.angularMomentum), inertiaTensorInverse(p.inertiaTensorInverse), inertiaTensorObj(p.inertiaTensorObj), torque(p.torque), totalMass(p.totalMass), vertices(p.vertices)
{
}

Physics::Physics(Physics&& p)
	: centerOfMass(p.centerOfMass), initCenterOfMass(p.initCenterOfMass), translation(p.translation), linearMomentum(p.linearMomentum), linearVelocity(p.linearVelocity), force(p.force), rotation(p.rotation), angularMomentum(p.angularMomentum), inertiaTensorInverse(p.inertiaTensorInverse), inertiaTensorObj(p.inertiaTensorObj), torque(p.torque), totalMass(p.totalMass), vertices(p.vertices)
{
}

Physics& Physics::operator=(const Physics& p)
{
	centerOfMass = p.centerOfMass;
	initCenterOfMass = p.initCenterOfMass;
	translation = p.translation;
	linearMomentum = p.linearMomentum;
	linearVelocity = p.linearVelocity;
	force = p.force;

	rotation = p.rotation;
	angularMomentum = p.angularMomentum;
	inertiaTensorInverse = p.inertiaTensorInverse;
	inertiaTensorObj = p.inertiaTensorObj;
	torque = p.torque;

	totalMass = p.totalMass;

	initVertices = p.initVertices;
	vertices = p.vertices;
	return *this;
}

Physics& Physics::operator=(Physics&& p)
{
	centerOfMass = p.centerOfMass;
	initCenterOfMass = p.initCenterOfMass;
	translation = p.translation;
	linearMomentum = p.linearMomentum;
	linearVelocity = p.linearVelocity;
	force = p.force;

	rotation = p.rotation;
	angularMomentum = p.angularMomentum;
	inertiaTensorInverse = p.inertiaTensorInverse;
	inertiaTensorObj = p.inertiaTensorObj;
	torque = p.torque;

	totalMass = p.totalMass;

	initVertices = p.initVertices;
	vertices = p.vertices;
	return *this;
}

Physics::~Physics()
{
}

void Physics::Initialize(glm::mat4 parentToBoneFromUnitMatrix)
{
	centerOfMass = glm::vec3(0.f, 0.f, 0.f);
	vertices = initVertices;
	glm::vec4 anchorPoint4 = parentToBoneFromUnitMatrix * glm::vec4(0.f, 0.f, 0.f, 1.f);
	glm::vec3 anchorPoint = glm::vec3(anchorPoint4.x, anchorPoint4.y, anchorPoint4.z);
	for (glm::vec3& vertex : vertices)
	{
		vertex = vertex - anchorPoint;
		centerOfMass += vertex;
	}

	totalMass = 1.f;
	if (vertices.empty() == false)
	{
		centerOfMass /= vertices.size();
	}
	initCenterOfMass = centerOfMass;

	// Calculate inertiaTensor
	rotation = glm::mat4(1.f);
	inertiaTensorObj = glm::mat3(0.f);
	for (const glm::vec3& vertex : vertices)
	{
		glm::vec3 ri = (vertex - centerOfMass);

		inertiaTensorObj[0][0] += (ri.y * ri.y + ri.z * ri.z) / vertices.size();
		inertiaTensorObj[1][1] += (ri.x * ri.x + ri.z * ri.z) / vertices.size();
		inertiaTensorObj[2][2] += (ri.x * ri.x + ri.y * ri.y) / vertices.size();

		inertiaTensorObj[0][1] -= (ri.x * ri.y) / vertices.size();

		inertiaTensorObj[0][2] -= (ri.x * ri.z) / vertices.size();

		inertiaTensorObj[1][2] -= (ri.y * ri.z) / vertices.size();
	}
	inertiaTensorObj[1][0] = inertiaTensorObj[0][1];
	inertiaTensorObj[2][0] = inertiaTensorObj[0][2];
	inertiaTensorObj[2][1] = inertiaTensorObj[1][2];
	
	if (vertices.empty() == false)
	{
		inertiaTensorInverse = rotation * glm::inverse(inertiaTensorObj) * glm::transpose(rotation);
	}
	else
	{
		inertiaTensorInverse = glm::mat3(0.f);
	}

	translation = glm::vec3(0.f);
	linearMomentum = glm::vec3(0.f);
	linearVelocity = glm::vec3(0.f);
	force = glm::vec3(0.f);

	angularMomentum = glm::vec3(0.f);
	angularVelocity = glm::vec3(0.f);
}

void Physics::UpdateByForce(float dt, glm::vec3 _force)
{
	force = _force;
	linearMomentum += dt * force;
	linearVelocity = linearMomentum / totalMass;
	translation += dt * linearVelocity;
	centerOfMass = initCenterOfMass + translation;
}

void Physics::UpdateByForce(float dt, glm::vec3 _force, glm::vec3 _torque)
{
	force = _force;
	torque = _torque;
	linearMomentum += dt * force;
	angularMomentum += dt * torque;
	linearVelocity = linearMomentum / totalMass;
	angularVelocity = inertiaTensorInverse * angularMomentum;
	translation += dt * linearVelocity;
	rotation += dt * (Tilde(angularVelocity) * rotation);

	centerOfMass = rotation * initCenterOfMass + translation;

	if (!vertices.empty())
	{
		inertiaTensorInverse = rotation * glm::inverse(inertiaTensorObj) * glm::transpose(rotation);
	}
}

glm::mat3 Physics::Tilde(glm::vec3 v)
{
	glm::mat3 result(0.f);

	result[1][0] = -v.z;
	result[2][0] = v.y;
	result[0][1] = v.z;
	result[2][1] = -v.x;
	result[0][2] = -v.y;
	result[1][2] = v.x;

	return result;
}
