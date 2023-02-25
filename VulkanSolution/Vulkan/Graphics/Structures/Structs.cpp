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

	std::vector<JiggleBone*> jiggleBones;
	jiggleBones.reserve(boneSize);

	for (auto it = bones.begin(); it != bones.end(); it++)
	{
		Bone* ptr = (*it);

		if (JiggleBone* jbPtr = dynamic_cast<JiggleBone*>(ptr);
			jbPtr != nullptr)
		{
			ptr->Update(dt);
			jiggleBones.push_back(jbPtr);
		}
	}

	for (auto it = jiggleBones.begin(); it != jiggleBones.end(); it++)
	{
		(*it)->UpdatePhysicsTransformations();
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
	: Bone(), isUpdateJigglePhysics(false), customPhysicsTranslation(glm::mat4(0.f)), customPhysicsRotation(glm::mat4(0.f)), physics(), parentBonePtr(nullptr), childBonePtr(nullptr)
{
}

JiggleBone::JiggleBone(std::string name, int parentID, int id, glm::mat4 toBoneFromUnit, glm::mat4 toModelFromBone, const Bone* parentBonePtr, const JiggleBone* childBonePtr)
	: Bone(name, parentID, id, toBoneFromUnit, toModelFromBone), isUpdateJigglePhysics(false), customPhysicsTranslation(glm::identity<glm::mat4>()), customPhysicsRotation(glm::identity<glm::mat4>()), physics(), parentBonePtr(parentBonePtr), childBonePtr(childBonePtr)
{
}

JiggleBone::JiggleBone(const JiggleBone& jb)
	: Bone(jb), isUpdateJigglePhysics(jb.isUpdateJigglePhysics), customPhysicsTranslation(jb.customPhysicsTranslation), customPhysicsRotation(jb.customPhysicsRotation), physics(jb.physics), parentBonePtr(jb.parentBonePtr), childBonePtr(jb.childBonePtr)
{
}

JiggleBone::JiggleBone(JiggleBone&& jb)
	: Bone(jb), isUpdateJigglePhysics(jb.isUpdateJigglePhysics), customPhysicsTranslation(jb.customPhysicsTranslation), customPhysicsRotation(jb.customPhysicsRotation), physics(jb.physics), parentBonePtr(jb.parentBonePtr), childBonePtr(jb.childBonePtr)
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

	// @@TODO: Implement If parent bone is not anchor?, (In other words, it is also a stick of spring-mass-damper system?)
			// @@ TODO: Final bug to accomplish the above goal, Adjust physics.centerOfMass!!!!!,,, current system does not modify centerofmass of multiple links

	glm::vec3 anchorPoint = GetInitialPointA();
	glm::vec3 exertedAnchorPoint = GetDynamicPointA();
	glm::vec3 exertedPoint = GetDynamicPointB();

	glm::vec3 parentEndStickPoint(anchorPoint);
	glm::vec3 parentPhysicsLinearVelocity = glm::vec3(0.f);
	if (const JiggleBone* parentJiggleBone = dynamic_cast<const JiggleBone*>(parentBonePtr);
		parentJiggleBone != nullptr)
	{
		parentEndStickPoint = parentJiggleBone->GetDynamicPointB();
		parentPhysicsLinearVelocity = parentJiggleBone->physics.pastVelocity;
	}
	
	// spring force
	glm::vec3 springForce = Physics::SpringScaler * (parentEndStickPoint - exertedAnchorPoint);
	glm::vec3 dampingForce = Physics::DampingScaler * (parentPhysicsLinearVelocity - physics.linearVelocity);

	glm::vec3 forceA = springForce + dampingForce + (0.5f * gravityForce);
	glm::vec3 forceB = (0.5f * gravityForce);

	if (childBonePtr != nullptr)
	{
		glm::vec3 springForceB = Physics::SpringScaler * (childBonePtr->GetDynamicPointA() - exertedPoint);
		glm::vec3 dampingForceB = Physics::DampingScaler * (childBonePtr->physics.linearVelocity - physics.linearVelocity);
		forceB += springForceB + dampingForceB;
	}
	//else
	//{
	//	glm::vec3 dampingForceB = Physics::DampingScaler * ( - physics.linearVelocity);
	//	forceB += dampingForceB;
	//}

	glm::vec3 finalForce = forceA + forceB;

	// the reason why used (anchorPoint - anchorPoint), (x - y), x is the position where exerted on.
	glm::vec3 torquePoint = (exertedPoint) - physics.centerOfMass;	// bSide
	glm::vec3 torquePoint2 = (exertedAnchorPoint) - physics.centerOfMass;	// aSide
	glm::vec3 torque = glm::cross(torquePoint, forceB);
	glm::vec3 torque2 = glm::cross(torquePoint2, springForce + dampingForce + (0.5f * gravityForce));
	if (Physics::forceApplyFlag)
	{
		physics.UpdateByForce(dt, finalForce, torque + torque2);
	}
	else
	{
		physics.UpdateByForce(dt, glm::vec3(0.f), glm::vec3(0.f));
	}
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
	childBonePtr = jb.childBonePtr;
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
	childBonePtr = jb.childBonePtr;
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
		physics.Initialize();
	}
}

void JiggleBone::AddVertices(const std::vector<glm::vec3>& _vertices)
{
	physics.initVertices = _vertices;
	physics.vertices = _vertices;
}

void JiggleBone::UpdatePhysicsTransformations()
{
	customPhysicsTranslation = glm::translate(physics.translation);
	customPhysicsRotation = glm::mat4(physics.rotation);
}

void JiggleBone::SetChildBonePtr(const JiggleBone* _childBonePtr)
{
	childBonePtr = _childBonePtr;
}

glm::vec3 JiggleBone::GetInitialPointA() const
{
	glm::vec4 bindPoseDifference = (parentBonePtr->toBoneFromUnit * glm::vec4(0.f, 0.f, 0.f, 1.f));
	glm::vec3 anchorPoint = glm::vec3(bindPoseDifference.x, bindPoseDifference.y, bindPoseDifference.z);

	return anchorPoint;
}

glm::vec3 JiggleBone::GetDynamicPointA() const
{
	glm::vec4 initPointA = (parentBonePtr->toBoneFromUnit * glm::vec4(0.f, 0.f, 0.f, 1.f));
	
	glm::vec4 result4 = CalculateParentTransformationRecursively(this, initPointA);

	return glm::vec3(result4.x, result4.y, result4.z);
}

glm::vec3 JiggleBone::GetDynamicPointB() const
{
	glm::vec4 initPointA = (toBoneFromUnit * glm::vec4(0.f, 0.f, 0.f, 1.f));

	glm::vec4 result4 = CalculateParentTransformationRecursively(this, initPointA);

	return glm::vec3(result4.x, result4.y, result4.z);
}

glm::vec4 JiggleBone::CalculateParentTransformationRecursively(const JiggleBone* jb, glm::vec4 firstGlobalPosition) const
{
	if (jb == nullptr)
	{
		return firstGlobalPosition;
	}

	const JiggleBone* parentJiggleBonePtr = dynamic_cast<const JiggleBone*>(jb->parentBonePtr);
	glm::vec4 result = firstGlobalPosition; // CalculateParentTransformationRecursively(parentJiggleBonePtr, firstGlobalPosition);

	glm::vec4 dynamicEndResult = jb->customPhysicsTranslation * glm::translate(jb->physics.centerOfMass) * jb->customPhysicsRotation * glm::translate(-jb->physics.centerOfMass) * result;

	return dynamicEndResult;
}

Physics::Physics()
	:centerOfMass(), initCenterOfMass(), translation(), linearMomentum(), linearVelocity(), pastVelocity(), force(), rotation(), angularMomentum(), inertiaTensorInverse(), inertiaTensorObj(), torque(), totalMass(), vertices()
{
}

Physics::Physics(const Physics& p)
	:centerOfMass(p.centerOfMass), initCenterOfMass(p.initCenterOfMass), translation(p.translation), linearMomentum(p.linearMomentum), linearVelocity(p.linearVelocity), pastVelocity(p.pastVelocity), force(p.force), rotation(p.rotation), angularMomentum(p.angularMomentum), inertiaTensorInverse(p.inertiaTensorInverse), inertiaTensorObj(p.inertiaTensorObj), torque(p.torque), totalMass(p.totalMass), vertices(p.vertices)
{
}

Physics::Physics(Physics&& p)
	: centerOfMass(p.centerOfMass), initCenterOfMass(p.initCenterOfMass), translation(p.translation), linearMomentum(p.linearMomentum), linearVelocity(p.linearVelocity), pastVelocity(p.pastVelocity), force(p.force), rotation(p.rotation), angularMomentum(p.angularMomentum), inertiaTensorInverse(p.inertiaTensorInverse), inertiaTensorObj(p.inertiaTensorObj), torque(p.torque), totalMass(p.totalMass), vertices(p.vertices)
{
}

Physics& Physics::operator=(const Physics& p)
{
	centerOfMass = p.centerOfMass;
	initCenterOfMass = p.initCenterOfMass;
	translation = p.translation;
	linearMomentum = p.linearMomentum;
	linearVelocity = p.linearVelocity;
	pastVelocity = p.pastVelocity;
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
	pastVelocity = p.pastVelocity;
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

void Physics::Initialize()
{
	centerOfMass = glm::vec3(0.f, 0.f, 0.f);
	vertices = initVertices;
	for (glm::vec3& vertex : vertices)
	{
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
	pastVelocity = glm::vec3(0.f);
	force = glm::vec3(0.f);

	angularMomentum = glm::vec3(0.f);
	angularVelocity = glm::vec3(0.f);
}

void Physics::UpdateByForce(float dt, glm::vec3 _force)
{
	force = _force;
	linearMomentum += dt * force;
	pastVelocity = linearVelocity;
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
	pastVelocity = linearVelocity;
	linearVelocity = linearMomentum / totalMass;
	angularVelocity = inertiaTensorInverse * angularMomentum;
	translation += dt * linearVelocity;
	rotation += dt * (Tilde(angularVelocity) * rotation);

	pastCOM = centerOfMass;
	centerOfMass = initCenterOfMass + translation;

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
