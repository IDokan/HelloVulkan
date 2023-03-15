#include "Structs.h"

bool Physics::forceApplyFlag = false;
float Physics::GravityScaler = 1.f;

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

void Skeleton::Update(float dt, glm::mat4 modelMatrix, bool bindPoseFlag, std::vector<glm::mat4>* animationMatrix)
{
	// Iterate bones but usually useful bones are at the tails.
	// It might be bad.
		// Probabily use reversed iterator for better performance.



	glm::vec4 result = normalize(glm::inverse(modelMatrix) * glm::vec4(0.f, -1.f, 0.f, 0.f));
	glm::vec3 gravityVector(result.x, result.y, result.z);

	std::vector<JiggleBone*> jiggleBones;
	jiggleBones.reserve(boneSize);

	for (auto it = bones.begin(); it != bones.end(); it++)
	{
		Bone* ptr = (*it);

		if (JiggleBone* jbPtr = dynamic_cast<JiggleBone*>(ptr);
			jbPtr != nullptr)
		{
			jiggleBones.push_back(jbPtr);
		}
	}

	const size_t jiggleBoneSize = jiggleBones.size();
	if (jiggleBoneSize <= 0)
	{
		return;
	}

	
	std::vector<JiggleBone> k1;
	CopyJiggleBoneVectors(jiggleBones, k1);
	std::vector<JiggleBone> k2, k3;
	CopyJiggleBoneVectors(k1, k2);
	CopyJiggleBoneVectors(k1, k3);
	

	std::vector<glm::vec3> linearForcesA1(jiggleBoneSize), linearForcesB1(jiggleBoneSize), torqueForces1(jiggleBoneSize);
	for (size_t i = 0; i < jiggleBoneSize; i++)
	{
		// 			ptr->Update(dt, gravityVector, bindPoseFlag, animationMatrix);
		k1[i].CalculateForces(linearForcesA1[i], linearForcesB1[i], torqueForces1[i]);
	}

	for (size_t i = 0; i < jiggleBoneSize; i++)
	{
		k1[i].UpdatePhysics(dt * 0.5f, { linearForcesA1[i] * 0.5f, linearForcesB1[i] * 0.5f }, torqueForces1[i] * 0.5f);
	}

	std::vector<glm::vec3> linearForcesA2(jiggleBoneSize), linearForcesB2(jiggleBoneSize), torqueForces2(jiggleBoneSize);
	for (size_t i = 0; i < jiggleBoneSize; i++)
	{
		k1[i].CalculateForces(linearForcesA2[i], linearForcesB2[i], torqueForces2[i]);
	}

	for (size_t i = 0; i < jiggleBoneSize; i++)
	{
		k2[i].UpdatePhysics(dt * 0.5f, { linearForcesA2[i] * 0.5f, linearForcesB2[i] * 0.5f }, torqueForces2[i] * 0.5f);
	}

	std::vector<glm::vec3> linearForcesA3(jiggleBoneSize), linearForcesB3(jiggleBoneSize), torqueForces3(jiggleBoneSize);
	for (size_t i = 0; i < jiggleBoneSize; i++)
	{
		k2[i].CalculateForces(linearForcesA3[i], linearForcesB3[i], torqueForces3[i]);
	}

	for (size_t i = 0; i < jiggleBoneSize; i++)
	{
		k3[i].UpdatePhysics(dt, { linearForcesA3[i],linearForcesB3[i] }, torqueForces3[i]);
	}

	std::vector<glm::vec3> linearForcesA4(jiggleBoneSize), linearForcesB4(jiggleBoneSize), torqueForces4(jiggleBoneSize);
	for (size_t i = 0; i < jiggleBoneSize; i++)
	{
		k3[i].CalculateForces(linearForcesA4[i], linearForcesB4[i], torqueForces4[i]);
	}

	for (size_t i = 0; i < jiggleBoneSize; i++)
	{
		jiggleBones[i]->UpdatePhysics(dt,
			{ (linearForcesA1[i] + (2.f * linearForcesA2[i]) + (2.f * linearForcesA3[i]) + linearForcesA4[i]) / 6.f,
			(linearForcesB1[i] + (2.f * linearForcesB2[i]) + (2.f * linearForcesB3[i]) + linearForcesB4[i]) / 6.f },
			(torqueForces1[i] + (2.f * torqueForces2[i]) + (2.f * torqueForces3[i]) + torqueForces4[i]) / 6.f);
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

void Skeleton::CleanBones()
{
	std::vector<JiggleBone*> jiggleBones;
	jiggleBones.reserve(boneSize);
	int numJiggleBone = 0;

	for (auto it = bones.begin(); it != bones.end(); it++)
	{
		Bone* ptr = (*it);

		if (JiggleBone* jbPtr = dynamic_cast<JiggleBone*>(ptr);
			jbPtr != nullptr)
		{
			jiggleBones.push_back(jbPtr);
			numJiggleBone++;
		}
	}

	for (JiggleBone* jb : jiggleBones)
	{
		delete jb;
	}

	boneSize -= numJiggleBone;

	bones.resize(boneSize);
}

void Skeleton::CopyJiggleBoneVectors(std::vector<JiggleBone*>& src, std::vector<JiggleBone>& dst)
{
	const size_t jiggleBoneSize = src.size();
	dst.resize(jiggleBoneSize);
	for (size_t i = 0; i < jiggleBoneSize; i++)
	{
		dst[i] = *src.at(i);
		if (i > 0)
		{
			dst[i - 1].childBonePtr = &dst[i];
			dst[i].parentBonePtr = &dst[i - 1];
		}
	}
}

void Skeleton::CopyJiggleBoneVectors(std::vector<JiggleBone>& src, std::vector<JiggleBone>& dst)
{
	const size_t jiggleBoneSize = src.size();
	dst.resize(jiggleBoneSize);
	for (size_t i = 0; i < jiggleBoneSize; i++)
	{
		dst[i] = src.at(i);
		if (i > 0)
		{
			dst[i - 1].childBonePtr = &dst[i];
			dst[i].parentBonePtr = &dst[i - 1];
		}
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

bool Bone::Update(float dt)
{
	return false;
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
	: Bone(), isUpdateJigglePhysics(false), customPhysicsTranslation(glm::mat4(0.f)), customPhysicsRotation(glm::mat4(1.f)), physics(), parentBonePtr(nullptr), childBonePtr(nullptr)
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

bool JiggleBone::Update(float dt)
{

	return true;
}

void JiggleBone::CalculateForces(glm::vec3& linearForceA, glm::vec3& linearForceB, glm::vec3& torqueForce, glm::vec3 gravityVector, bool bindPoseFlag, std::vector<glm::mat4>* animationMatrix)
{
	if (isUpdateJigglePhysics == false)
	{
		linearForceA = glm::vec3(0.f);
		linearForceB = glm::vec3(0.f);
		torqueForce = glm::vec3(0.f);
		return;
	}

	// @@ Begin of Physics calculation
	glm::vec3 gravityForce = gravityVector * Physics::GravityScaler;

	// @@TODO: Implement If parent bone is not anchor?, (In other words, it is also a stick of spring-mass-damper system?)
			// @@ TODO: Final bug to accomplish the above goal, Adjust physics.centerOfMass!!!!!,,, current system does not modify centerofmass of multiple links

	glm::vec3 anchorPoint = GetInitialPointA(true, animationMatrix);
	glm::vec3 exertedAnchorPoint = GetDynamicPointA(true, animationMatrix);
	glm::vec3 exertedPoint = GetDynamicPointB(true, animationMatrix);

	glm::vec3 parentEndStickPoint(anchorPoint);
	glm::vec3 parentPhysicsLinearVelocity = glm::vec3(0.f);
	if (const JiggleBone* parentJiggleBone = dynamic_cast<const JiggleBone*>(parentBonePtr);
		parentJiggleBone != nullptr)
	{
		parentEndStickPoint = parentJiggleBone->GetDynamicPointB(true, animationMatrix);
		parentPhysicsLinearVelocity = parentJiggleBone->physics.linearVelocities[1];
	}

	// spring force
	glm::vec3 springForce = physics.springScaler * (parentEndStickPoint - exertedAnchorPoint);
	glm::vec3 dampingForce = physics.dampingScaler * (parentPhysicsLinearVelocity - physics.linearVelocities[0]);

	glm::vec3 forceA = springForce + dampingForce + (0.5f * gravityForce);
	glm::vec3 forceB = (0.5f * gravityForce);

	if (childBonePtr != nullptr)
	{
		glm::vec3 springForceB = childBonePtr->physics.springScaler * (childBonePtr->GetDynamicPointA(true, animationMatrix) - exertedPoint);
		glm::vec3 dampingForceB = childBonePtr->physics.dampingScaler * (childBonePtr->physics.linearVelocities[0] - physics.linearVelocities[1]);
		forceB += springForceB + dampingForceB;
	}

	linearForceA = forceA;
	linearForceB = forceB;

	// the reason why used (anchorPoint - anchorPoint), (x - y), x is the position where exerted on.
	glm::vec3 torquePoint = (exertedPoint)-physics.centerOfMass;	// bSide
	glm::vec3 torquePoint2 = (exertedAnchorPoint)-physics.centerOfMass;	// aSide
	glm::vec3 torque = glm::cross(torquePoint, forceB);
	glm::vec3 torque2 = glm::cross(torquePoint2, forceA);
	torqueForce = torque + torque2;
}

void JiggleBone::UpdatePhysics(float dt, const std::vector<glm::vec3>& linearForces, glm::vec3 torqueForce)
{
	if (Physics::forceApplyFlag)
	{
		physics.UpdateByForce(dt, linearForces, torqueForce);
		customPhysicsTranslation = glm::translate(physics.translation);
		customPhysicsRotation = glm::mat4(physics.rotation);
	}
	else
	{
		std::vector<glm::vec3> tmp(0);
		physics.UpdateByForce(dt, tmp, glm::vec3(0.f));
		customPhysicsTranslation = glm::translate(physics.translation);
		customPhysicsRotation = glm::mat4(physics.rotation);
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

void JiggleBone::SetChildBonePtr(const JiggleBone* _childBonePtr)
{
	childBonePtr = _childBonePtr;
}

glm::vec3 JiggleBone::GetInitialPointA(bool bindPoseFlag, std::vector<glm::mat4>* animationMatrix) const
{
	glm::vec4 bindPoseDifference;
	if (bindPoseFlag)
	{
		bindPoseDifference = (parentBonePtr->toBoneFromUnit * glm::vec4(0.f, 0.f, 0.f, 1.f));
	}
	else
	{
		bindPoseDifference = (animationMatrix->at(id) * parentBonePtr->toBoneFromUnit * glm::vec4(0.f, 0.f, 0.f, 1.f));
	}
	glm::vec3 anchorPoint = glm::vec3(bindPoseDifference.x, bindPoseDifference.y, bindPoseDifference.z);

	return anchorPoint;
}

glm::vec3 JiggleBone::GetDynamicPointA(bool bindPoseFlag, std::vector<glm::mat4>* animationMatrix) const
{
	glm::vec4 initPointA; 
	if (bindPoseFlag)
	{
		initPointA = (parentBonePtr->toBoneFromUnit * glm::vec4(0.f, 0.f, 0.f, 1.f));
	}
	else
	{
		initPointA = (animationMatrix->at(id) * parentBonePtr->toBoneFromUnit * glm::vec4(0.f, 0.f, 0.f, 1.f));
	}
	
	glm::vec4 result4 = CalculateParentTransformationRecursively(this, initPointA);

	return glm::vec3(result4.x, result4.y, result4.z);
}

glm::vec3 JiggleBone::GetDynamicPointB(bool bindPoseFlag, std::vector<glm::mat4>* animationMatrix) const
{
	glm::vec4 initPointA; 
	if (bindPoseFlag)
	{
		initPointA = (toBoneFromUnit * glm::vec4(0.f, 0.f, 0.f, 1.f));
	}
	else
	{
		initPointA = (animationMatrix->at(id) * toBoneFromUnit * glm::vec4(0.f, 0.f, 0.f, 1.f));
	}

	glm::vec4 result4 = CalculateParentTransformationRecursively(this, initPointA);

	return glm::vec3(result4.x, result4.y, result4.z);
}

glm::vec4 JiggleBone::CalculateParentTransformationRecursively(const JiggleBone* jb, glm::vec4 firstGlobalPosition) const
{
	if (jb == nullptr)
	{
		return firstGlobalPosition;
	}

	glm::vec4 result = firstGlobalPosition;

	glm::vec4 dynamicEndResult = jb->customPhysicsTranslation * glm::translate(jb->physics.centerOfMass) * jb->customPhysicsRotation * glm::translate(-jb->physics.centerOfMass) * result;

	return dynamicEndResult;
}

Physics::Physics()
	:centerOfMass(), initCenterOfMass(), translation(), linearMomentums(springSize, glm::vec3(0.f)), linearVelocities(springSize, glm::vec3(0.f)), force(), rotation(glm::mat3(1.f)), angularMomentum(), inertiaTensorInverse(), inertiaTensorObj(), torque(), totalMass(), vertices(),
	dampingScaler(5.f), springScaler(50.f)
{
}

Physics::Physics(const Physics& p)
	:centerOfMass(p.centerOfMass), initCenterOfMass(p.initCenterOfMass), translation(p.translation), linearMomentums(p.linearMomentums), linearVelocities(p.linearVelocities), force(p.force), rotation(p.rotation), angularMomentum(p.angularMomentum), inertiaTensorInverse(p.inertiaTensorInverse), inertiaTensorObj(p.inertiaTensorObj), torque(p.torque), totalMass(p.totalMass), vertices(p.vertices),
	dampingScaler(p.dampingScaler), springScaler(p.springScaler)
{
}

Physics::Physics(Physics&& p)
	: centerOfMass(p.centerOfMass), initCenterOfMass(p.initCenterOfMass), translation(p.translation), linearMomentums(p.linearMomentums), linearVelocities(p.linearVelocities), force(p.force), rotation(p.rotation), angularMomentum(p.angularMomentum), inertiaTensorInverse(p.inertiaTensorInverse), inertiaTensorObj(p.inertiaTensorObj), torque(p.torque), totalMass(p.totalMass), vertices(p.vertices),
	dampingScaler(p.dampingScaler), springScaler(p.springScaler)
{
}

Physics& Physics::operator=(const Physics& p)
{
	centerOfMass = p.centerOfMass;
	initCenterOfMass = p.initCenterOfMass;
	translation = p.translation;
	for (size_t i = 0; i < springSize; i++)
	{
		linearMomentums[i] = p.linearMomentums[i];
		linearVelocities[i] = p.linearVelocities[i];
	}
	force = p.force;

	rotation = p.rotation;
	angularMomentum = p.angularMomentum;
	inertiaTensorInverse = p.inertiaTensorInverse;
	inertiaTensorObj = p.inertiaTensorObj;
	torque = p.torque;

	totalMass = p.totalMass;

	initVertices = p.initVertices;
	vertices = p.vertices;

	dampingScaler = p.dampingScaler;
	springScaler = p.springScaler;

	return *this;
}

Physics& Physics::operator=(Physics&& p)
{
	centerOfMass = p.centerOfMass;
	initCenterOfMass = p.initCenterOfMass;
	translation = p.translation;
	for (size_t i = 0; i < springSize; i++)
	{
		linearMomentums[i] = p.linearMomentums[i];
		linearVelocities[i] = p.linearVelocities[i];
	}
	force = p.force;

	rotation = p.rotation;
	angularMomentum = p.angularMomentum;
	inertiaTensorInverse = p.inertiaTensorInverse;
	inertiaTensorObj = p.inertiaTensorObj;
	torque = p.torque;

	totalMass = p.totalMass;

	initVertices = p.initVertices;
	vertices = p.vertices;

	dampingScaler = p.dampingScaler;
	springScaler = p.springScaler;

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
	linearMomentums.resize(springSize);
	linearVelocities.resize(springSize);
	for (size_t i = 0; i < springSize; i++)
	{
		linearMomentums[i] = glm::vec3(0.f);
		linearVelocities[i] = glm::vec3(0.f);
	}
	force = glm::vec3(0.f);
	torque = glm::vec3(0.f);

	angularMomentum = glm::vec3(0.f);
	angularVelocity = glm::vec3(0.f);
}

void Physics::UpdateByForce(float dt, glm::vec3 _force)
{
	abort();
	// deprecated function. abort when it called.
	force = _force;
	linearMomentums[0] += dt * force;
	linearVelocities[0] = linearMomentums[0] / totalMass;
	translation += dt * linearVelocities[0];
	centerOfMass = initCenterOfMass + translation;
}

void Physics::UpdateByForce(float dt, std::vector<glm::vec3> _forces, glm::vec3 _torque)
{
	const size_t forceSize = _forces.size();
	force = glm::vec3(0.f);
	for (size_t i = 0; i < forceSize; i++)
	{
		force += _forces[i];
	}
	torque = _torque;
	for (size_t i = 0; i < forceSize; i++)
	{
		linearMomentums[i] += dt * _forces[i];
	}
	angularMomentum += dt * torque;
	glm::vec3 velocitySum = glm::vec3(0.f);
	for (size_t i = 0; i < forceSize; i++)
	{
		linearVelocities[i] = linearMomentums[i] / totalMass;
		velocitySum += linearVelocities[i];
	}
	angularVelocity = inertiaTensorInverse * angularMomentum;
	translation += dt * (velocitySum);
	rotation += dt * (Tilde(angularVelocity) * rotation);

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
