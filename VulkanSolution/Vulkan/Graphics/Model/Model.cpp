/******************************************************************************
Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   Model.cpp
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 09.27.2022
	Source file for custom structures.
******************************************************************************/

#include <iostream>
#include "Graphics/Model/Model.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "stb/stb_image.h"
#include "Graphics/Model/AnimationSystem.h"


Model::Model(const std::string& path)
	: isModelValid(true), lSdkManager(nullptr), ios(nullptr), lImporter(nullptr), animationSystem(nullptr)
{
	animationSystem = new AnimationSystem();
	LoadModel(path);
}

Model::~Model()
{
	delete animationSystem;
	CleanFBXResources();
}

bool Model::LoadModel(const std::string& path)
{
	ClearData();
	CleanFBXResources();

	// It handles memory management.
	lSdkManager = FbxManager::Create();

	ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
	lSdkManager->SetIOSettings(ios);

	lImporter = FbxImporter::Create(lSdkManager, "");

	if (!lImporter->Initialize(path.c_str(), -1, ios))
	{
		isModelValid = false;
		return false;
	}

	lScene = FbxScene::Create(lSdkManager, "myScene");

	lImporter->Import(lScene);

	GetSkeleton();
	GetScene();
	GetAnimation();

	InitBoneData();

	lImporter->Destroy();
	lImporter = nullptr;


	//meshes.clear();

	//const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenNormals);

	//if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	//{
	//	isModelValid = false;
	//	return isModelValid;
	//}

	//if (scene->HasMeshes())
	//{
	//	ReadMesh(scene->mRootNode, scene);
	//}
	//if (scene->HasMaterials())
	//{
	//	ReadMaterial(scene, path);
	//}

	isModelValid = true;
	return isModelValid;
}

void Model::Update(float dt, glm::mat4 modelMatrix, bool bindPoseFlag)
{
	animationSystem->Update(dt, modelMatrix, bindPoseFlag, &animationMatrix);
}

void Model::CleanBones()
{
	animationSystem->CleanBones();
}

int Model::GetMeshSize()
{
	return static_cast<int>(meshes.size());
}

std::string Model::GetMeshName(int i)
{
	return meshes[i].meshName;
}

void* Model::GetVertexData(int i)
{
	return reinterpret_cast<void*>(meshes[i].vertices.data());
}

int Model::GetVertexCount(int i)
{
	return static_cast<int>(meshes[i].vertices.size());
}

void* Model::GetIndexData(int i)
{
	return reinterpret_cast<void*>(meshes[i].indices.data());
}

int Model::GetIndexCount(int i)
{
	return static_cast<int>(meshes[i].indices.size());
}

void* Model::GetUniqueVertexData(int i)
{
	return reinterpret_cast<void*>(meshes[i].uniqueVertices.data());
}

int Model::GetUniqueVertexCount(int i)
{
	return static_cast<int>(meshes[i].uniqueVertices.size());
}

bool Model::IsModelValid()
{
	return isModelValid;
}

size_t Model::GetBoneCount()
{
	return animationSystem->GetBoneCount();
}

void* Model::GetBoneDataForDrawing()
{
	return reinterpret_cast<void*>(bones.data());
}

const char* Model::GetErrorString()
{
	return lImporter->GetStatus().GetErrorString();
}

void Model::GetBoundingBoxMinMax(glm::vec3& min, glm::vec3& max)
{
	min = boundingBox[0];
	max = boundingBox[1];
}

glm::mat4 Model::CalculateAdjustBoundingBoxMatrix()
{
	glm::vec3 modelScale = GetModelScale();
	float largest = std::max(std::max(modelScale.x, modelScale.y), modelScale.z);
	return glm::scale(glm::vec3(2.f / largest)) * glm::translate(-GetModelCentroid());
}

const std::vector<std::string>& Model::GetDiffuseImagePaths()
{
	return diffuseImagePaths;
}

const std::vector<std::string>& Model::GetNormalImagePaths()
{
	return normalImagePaths;
}

void Model::ChangeBoneIndexInSphere(int meshIndex, glm::vec3 trans, float radius, int boneIDIndex, int newBoneIndex, glm::vec4 weight, std::vector<glm::vec3>& changedVertices)
{
	Mesh& mesh = meshes[meshIndex];
	float radiusSquared = radius * radius;
	changedVertices.clear();
	for (Vertex& vertex : mesh.vertices)
	{
		glm::vec3 diff = vertex.position - trans;
		float lengthSquared = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
		if (lengthSquared <= radiusSquared)
		{
			vertex.boneIDs[boneIDIndex] = newBoneIndex;
			vertex.boneWeights = weight;
			changedVertices.push_back(vertex.position);
		}
	}
	for (Vertex& vertex : mesh.uniqueVertices)
	{
		glm::vec3 diff = vertex.position - trans;
		float lengthSquared = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
		if (lengthSquared <= radiusSquared)
		{
			vertex.boneIDs[boneIDIndex] = newBoneIndex;
			vertex.boneWeights = weight;
		}
	}
}

void Model::ExploreScene(FbxScene* scene)
{
	// The first root node is always empty
	FbxNode* lRootNode = scene->GetRootNode();

	if (lRootNode)
	{
		for (int i = 0; i < lRootNode->GetChildCount(); i++)
		{
			PrintNode(lRootNode->GetChild(i));
		}
	}
}

void Model::PrintNode(FbxNode* node)
{
	PrintTabs();
	const char* nodeName = node->GetName();
	FbxDouble3 translation = node->LclTranslation.Get();
	FbxDouble3 rotation = node->LclRotation.Get();
	FbxDouble3 scaling = node->LclScaling.Get();

	// Print the contents of the node.
	printf("<node name='%s' translation='(%f, %f, %f)' rotation='(%f, %f, %f)' scaling='(%f, %f, %f)'>\n",
		nodeName,
		translation[0], translation[1], translation[2],
		rotation[0], rotation[1], rotation[2],
		scaling[0], scaling[1], scaling[2]
	);
	numTabs++;

	// Print the node's attributes.
	for (int i = 0; i < node->GetNodeAttributeCount(); i++)
		PrintAttribute(node->GetNodeAttributeByIndex(i));

	// Recursively print the children.
	for (int j = 0; j < node->GetChildCount(); j++)
		PrintNode(node->GetChild(j));

	numTabs--;
	PrintTabs();
	printf("</node>\n");
}

void Model::PrintTabs()
{
	for (int i = 0; i < numTabs; i++)
		printf("\t");
}

void Model::PrintAttribute(FbxNodeAttribute* attribute)
{
	if (!attribute) return;

	FbxString typeName = GetAttributeTypeName(attribute->GetAttributeType());
	FbxString attrName = attribute->GetName();
	PrintTabs();
	// Note: to retrieve the character array of a FbxString, use its Buffer() method.
	printf("<attribute type='%s' name='%s'/>\n", typeName.Buffer(), attrName.Buffer());
}

FbxString Model::GetAttributeTypeName(FbxNodeAttribute::EType type)
{
	switch (type) {
	case FbxNodeAttribute::eUnknown: return "unidentified";
	case FbxNodeAttribute::eNull: return "null";
	case FbxNodeAttribute::eMarker: return "marker";
	case FbxNodeAttribute::eSkeleton: return "skeleton";
	case FbxNodeAttribute::eMesh: return "mesh";
	case FbxNodeAttribute::eNurbs: return "nurbs";
	case FbxNodeAttribute::ePatch: return "patch";
	case FbxNodeAttribute::eCamera: return "camera";
	case FbxNodeAttribute::eCameraStereo: return "stereo";
	case FbxNodeAttribute::eCameraSwitcher: return "camera switcher";
	case FbxNodeAttribute::eLight: return "light";
	case FbxNodeAttribute::eOpticalReference: return "optical reference";
	case FbxNodeAttribute::eOpticalMarker: return "marker";
	case FbxNodeAttribute::eNurbsCurve: return "nurbs curve";
	case FbxNodeAttribute::eTrimNurbsSurface: return "trim nurbs surface";
	case FbxNodeAttribute::eBoundary: return "boundary";
	case FbxNodeAttribute::eNurbsSurface: return "nurbs surface";
	case FbxNodeAttribute::eShape: return "shape";
	case FbxNodeAttribute::eLODGroup: return "lodgroup";
	case FbxNodeAttribute::eSubDiv: return "subdiv";
	default: return "unknown";
	}
}

void Model::GetScene(FbxNode* root)
{
	if (!root)
	{
		root = lScene->GetRootNode();
		if (!root)
		{
			return;
		}
	}

	const uint32_t numNodes = static_cast<uint32_t>(root->GetChildCount());
	for (uint32_t i = 0; i < numNodes; i++)
	{
		FbxNode* node = root->GetChild(i);

		if (!node)
		{
			continue;
		}

		// If mesh data is valid,
		if (node->GetMesh())
		{
			GetMesh(node);
		}
		else
		{
			GetScene(node);
		}

	}
}

void Model::GetMesh(FbxNode* node)
{
	FbxMesh* mesh = node->GetMesh();
	if (!mesh)
	{
		return;
	}

	if (mesh->RemoveBadPolygons() < 0)
	{
		return;
	}

	// Triangulate the mesh if needed.
	FbxGeometryConverter gc{ lSdkManager };

	mesh = static_cast<FbxMesh*>(gc.Triangulate(mesh, true));


	if (!mesh || mesh->RemoveBadPolygons() < 0)
	{
		return;
	}

	Mesh m;
	m.meshName = (node->GetName()[0] != '\0') ? node->GetName() : mesh->GetName();

	animationSystem->GetDeformerData(mesh);

	if (GetMeshData(mesh, m))
	{
		meshes.emplace_back(m);
	}


	int materialCount = node->GetSrcObjectCount<FbxSurfaceMaterial>();
	if (materialCount > 0)
	{
		for (int i = 0; i < materialCount; i++)
		{
			FbxSurfaceMaterial* material = node->GetSrcObject<FbxSurfaceMaterial>(i);
			GetTextureData(material);
		}
	}

}

bool Model::GetMeshData(FbxMesh* mesh, Mesh& m)
{
	// 현재 복수의 중복되는 Vertex Data가 저장되어 있음
	// 그래서 특정 Vertex를 선택해도 중복되는 vertex가 덧씌어버려서 시각적으로 feedback이 없음
		// Solutions
		// 1. 개인 vertex들만 관리하는 data structure를 하나 추가한다.
		//			// 걱정되는 추후 발생 예상 문제점: 선택된 Vertex들만 physics를 적용시켰을 때 어떻게 해야할지 모르겠음.
		// 2. Change Mesh Import method.
	const uint32_t polygonCount = mesh->GetPolygonCount();
	if (polygonCount <= 0)
	{
		return false;
	}

	// @@ Get Vertices
	const uint32_t verticesCount = mesh->GetControlPointsCount();
	FbxVector4* vertices = mesh->GetControlPoints();
	m.uniqueVertices.resize(verticesCount);
	for (size_t i = 0; i < verticesCount; i++)
	{
		FbxVector4 v = vertices[i];
		m.uniqueVertices[i].position = glm::vec3(v[0], v[1], v[2]);
		glm::ivec4 boneID = animationSystem->GetBoneIndex(i);
		m.uniqueVertices[i].boneIDs = boneID;
		glm::vec4 boneWeights = animationSystem->GetBoneWeight(i);
		float sum = 0.f;
		for (int x = 0; x < 4; x++)
		{
			sum += boneWeights[x];
		}
		boneWeights /= sum;
		m.uniqueVertices[i].boneWeights = boneWeights;
	}
	const uint32_t indicesCount = mesh->GetPolygonVertexCount();
	int* indices = mesh->GetPolygonVertices();
	// @@ End of getting vertices


	// @@ Import normals
	FbxArray<FbxVector4> normals;
	// Calculate normals using FBX's built-in method, but only if no normal data is already there.

	mesh->GenerateNormals();
	mesh->GetPolygonVertexNormals(normals);

	const uint32_t normalCount = normals.Size();
	// @@ End of importing normals

	// @@ Import Texture Coordinates
	FbxStringList uvNames;
	mesh->GetUVSetNames(uvNames);
	const int uvSetCount = uvNames.GetCount();
	FbxArray<FbxVector2> uvs;
	int uvCount = 0;
	// for (int i = 0; i < uvSetCount; i++)
	{
		if (mesh->GetPolygonVertexUVs(uvNames.GetStringAt(0), uvs))
		{
			std::cout << uvNames.GetStringAt(0);
			uvCount = uvs.Size();
		}
	}
	// @@ End of importing Texture Coordinates


	if (!(verticesCount > 0 && vertices && indicesCount > 0 && indices && normalCount > 0))
	{
		return false;
	}

	m.indices.resize(indicesCount);
	// small vertices but more normals and uv data.
	// Expand vertices
	if (normalCount == indicesCount && uvCount == indicesCount)
	{
		for (size_t i = 0; i < indicesCount; i++)
		{
			int iInt = static_cast<int>(i);
			FbxVector4 v = vertices[indices[i]];
			m.indices[i] = static_cast<uint32_t>(m.vertices.size());
			m.vertices.emplace_back();
			glm::vec3 position = glm::vec3(v[0], v[1], v[2]);
			glm::vec3 normal = glm::vec3(normals[iInt][0], normals[iInt][1], normals[iInt][2]);
			m.normalByVertex[position].push_back(normal);
			glm::vec2 uv = glm::vec2(uvs[iInt][0], uvs[iInt][1]);
			m.uvByVertex[position].push_back(uv);
			glm::ivec4 boneID = animationSystem->GetBoneIndex(indices[i]);
			glm::vec4 boneWeights = animationSystem->GetBoneWeight(indices[i]);
			float sum = 0.f;
			for (int x = 0; x < 4; x++)
			{
				sum += boneWeights[x];
			}
			boneWeights /= sum;
			Vertex& vertex = m.vertices.back();
			vertex.position = position;
			vertex.normal = normal;
			vertex.texCoord = uv;
			vertex.boneIDs = boneID;
			vertex.boneWeights = boneWeights;

			UpdateBoundingBox(position);
		}
	}
	else
	{
		// When normal has same amount as positions
		const int invalidIndexValue = std::numeric_limits<unsigned int>::max();
		std::vector<unsigned int> vertexRef(verticesCount, invalidIndexValue);

		for (size_t i = 0; i < indicesCount; i++)
		{
			const int vertexIndex = indices[i];
			// Did we encounter this vertex before? If so, just add its index.
			// If not, add the vertex and a new index.
			if (vertexRef[vertexIndex] != invalidIndexValue)
			{
				m.indices[i] = vertexRef[vertexIndex];
			}
			else
			{
				FbxVector4 v = vertices[vertexIndex];
				m.indices[i] = static_cast<uint32_t>(m.vertices.size());
				vertexRef[vertexIndex] = m.indices[i];
				m.vertices.emplace_back();
				glm::vec3 position = glm::vec3(v[0], v[1], v[2]);
				m.vertices.back().position = position;

				UpdateBoundingBox(position);
			}
		}
	}

	if (m.indices.size() % 3 != 0)
	{
		// Invalid index size
		return false;
	}

	return true;
}

void Model::GetSkeleton(FbxNode* root)
{
	if (!root)
	{
		root = lScene->GetRootNode();
		if (!root)
		{
			return;
		}

		animationSystem->ImportSkeleton(root);
	}

	const uint32_t numNodes = static_cast<uint32_t>(root->GetChildCount());
	for (uint32_t i = 0; i < numNodes; i++)
	{
		FbxNode* node = root->GetChild(i);

		if (!node)
		{
			continue;
		}

		animationSystem->ImportSkeleton(node, root);

		GetSkeleton(node);
	}
}

void Model::InitBoneData()
{
	std::vector<glm::mat4> toBoneFromUnit;
	animationSystem->GetToBoneFromUnit(toBoneFromUnit);

	size_t boneCount = animationSystem->GetBoneCount();
	bones.resize(boneCount * 2);

	for (int i = 0; i < boneCount; i++)
	{
		// @@ Bone to Child bone
		bones[i * 2] = toBoneFromUnit[i] * glm::vec4(0.f, 0.f, 0.f, 1.f);
		int childrenID = animationSystem->GetChildrenBoneID(i);
		if (childrenID < 0)
		{
			bones[i * 2 + 1] = toBoneFromUnit[i] * glm::vec4(0.f, 1.f, 0.f, 1.f);
		}
		else
		{
			bones[i * 2 + 1] = toBoneFromUnit[childrenID] * glm::vec4(0.f, 0.f, 0.f, 1.f);
		}
	}
}

void Model::GetAnimation()
{
	FbxNode* rootNode = lScene->GetRootNode();

	if (rootNode == nullptr)
	{
		return;
	}

	double frameRate = FbxTime::GetFrameRate(lScene->GetGlobalSettings().GetTimeMode());
	FbxDocument* document = dynamic_cast<FbxDocument*>(lScene);

	if (document == nullptr)
	{
		return;
	}

	FbxArray<FbxString*> animNameArray;
	document->FillAnimStackNameArray(animNameArray);

	const int animStackCount = lImporter->GetAnimStackCount();
	for (int i = 0; i < animStackCount; i++)
	{
		// Take is technically an Animation. It's an Autodesk naming convention.
		FbxTakeInfo* animationInfo = lImporter->GetTakeInfo(i);
		std::string animationName = animationInfo->mName.Buffer();

		FbxTimeSpan span = animationInfo->mLocalTimeSpan;
		double startTime = span.GetStart().GetSecondDouble();
		double endTime = span.GetStop().GetSecondDouble();

		// This method interpolate transform data at loading.
		// It may not give you the specific key frames determined by artists.
		// Thus, key frame is not the key frame given by artists, it is interpolated key frame by multiplication between running time and frame rate.
		int keyFrames = static_cast<int>((endTime - startTime) * frameRate);

		if (keyFrames <= 1)
		{
			continue;
		}

		animationSystem->AddAnimation(animationName, animationSystem->GetBoneCount(), static_cast<float>(endTime - startTime));
		animationSystem->SetAnimationIndex(i);
		// When animation has a time to run, 
		if (startTime < endTime)
		{

			AddTracksRecursively(rootNode, frameRate, startTime, endTime, keyFrames);
		}
	}
}

void Model::AddTracksRecursively(FbxNode* node, double frameRate, double startTime, double endTime, int keyFrames)
{
	if (node == nullptr)
	{
		return;
	}

	animationSystem->AddTrack(node, animationSystem->GetBoneIDByName(node->GetName()), frameRate, startTime, endTime, keyFrames);

	int childCount = node->GetChildCount();
	for (int i = 0; i < childCount; i++)
	{
		AddTracksRecursively(node->GetChild(i), frameRate, startTime, endTime, keyFrames);
	}
}

void Model::GetTextureData(FbxSurfaceMaterial* material)
{
	if (material == nullptr)
	{
		return;
	}

	FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);

	// Directly get textures (Only diffuse)
	int textureFileCount = prop.GetSrcObjectCount<FbxFileTexture>();
	if (textureFileCount > 0)
	{
		FbxFileTexture* textureFile = FbxCast<FbxFileTexture>(prop.GetSrcObject<FbxFileTexture>(0));
		diffuseImagePaths.push_back(textureFile->GetFileName());
	}

}

void Model::ClearData()
{
	meshes.clear();
	bones.clear();
	diffuseImagePaths.clear();
	normalImagePaths.clear();

	animationSystem->Clear();

	boundingBox[0] = glm::vec3(INFINITY);
	boundingBox[1] = glm::vec3(-INFINITY);
}

void Model::CleanFBXResources()
{
	if (lImporter != nullptr)
	{
		lImporter->Destroy();
		lImporter = nullptr;
	}

	if (ios != nullptr)
	{
		ios->Destroy();
		ios = nullptr;
	}

	if (lSdkManager != nullptr)
	{
		lSdkManager->Destroy();
		lSdkManager = nullptr;
	}
}

void Model::ReadMesh(aiNode* node, const aiScene* scene)
{

	// Process all the node's meshes (if any)
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		unsigned int vertexSize = 0;
		for (int i = 0; i < meshes.size(); i++)
		{
			vertexSize += static_cast<unsigned int>(meshes[i].vertices.size());
		}
		Mesh m;
		unsigned int baseIndex = vertexSize;
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		for (unsigned int j = 0; j < mesh->mNumVertices; j++)
		{
			Vertex vertex;
			vertex.position = glm::vec3(mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z);

			if (mesh->HasNormals())
			{
				vertex.normal = glm::vec3(mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z);
			}

			// Temporary texture coordinate data
			if (mesh->HasTextureCoords(0))
			{
				vertex.texCoord = glm::vec2(mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y);
			}
			m.vertices.push_back(vertex);
			UpdateBoundingBox(m.vertices.back().position);
		}
		for (unsigned int j = 0; j < mesh->mNumFaces; j++)
		{
			aiFace face = mesh->mFaces[j];
			for (unsigned int k = 0; k < face.mNumIndices; k++)
			{
				m.indices.push_back(face.mIndices[k] + baseIndex);
			}
		}
		meshes.push_back(m);
	}


	// then do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ReadMesh(node->mChildren[i], scene);
	}
}

void Model::UpdateBoundingBox(glm::vec3 vertex)
{
	boundingBox[0].x = std::min(boundingBox[0].x, vertex.x);
	boundingBox[0].y = std::min(boundingBox[0].y, vertex.y);
	boundingBox[0].z = std::min(boundingBox[0].z, vertex.z);
	boundingBox[1].x = std::max(boundingBox[1].x, vertex.x);
	boundingBox[1].y = std::max(boundingBox[1].y, vertex.y);
	boundingBox[1].z = std::max(boundingBox[1].z, vertex.z);
}

std::string Model::GetBoneName(unsigned int boneID)
{
	return animationSystem->GetBoneName(boneID);
}

const Bone* Model::GetBone(unsigned int boneID)
{
	return animationSystem->GetBone(boneID);
}

glm::vec3 Model::GetModelScale()
{
	return boundingBox[1] - boundingBox[0];
}

void Model::GetToBoneFromUnit(std::vector<glm::mat4>& data)
{
	animationSystem->GetToBoneFromUnit(data);
}

unsigned int Model::GetAnimationCount()
{
	return animationSystem->GetAnimationCount();
}

unsigned int Model::GetSelectedAnimationIndex()
{
	return animationSystem->GetSelectedAnimationIndex();
}

void Model::SetAnimationIndex(unsigned int i)
{
	if (const unsigned int animationCount = animationSystem->GetAnimationCount();
		i >= animationCount)
	{
		i = animationCount - 1;
	}
	animationSystem->SetAnimationIndex(i);
}

void Model::CalculateAnimation(float t, bool bindPoseFlag)
{
	animationMatrix.clear();

	if (bindPoseFlag || animationSystem->GetAnimationCount() <= 0)
	{
		size_t boneCount = GetBoneCount();
		animationMatrix.resize(boneCount);

		for (int i = 0; i < boneCount; i++)
		{
			animationMatrix[i] = glm::identity<glm::mat4>();

			if (int pID = animationSystem->GetBone(i)->parentID;
				pID >= 0)
			{
				animationMatrix[i] = animationMatrix[i] * animationMatrix[pID];
			}
		}


		for (int i = 0; i < boneCount; i++)
		{
			// Apply physics matrix in here for bind pose.
			if (const JiggleBone* jb = dynamic_cast<const JiggleBone*>(animationSystem->GetBone(i));
				jb != nullptr)
			{
				glm::vec3 vertexPos = jb->physics.pastCOM;
				animationMatrix[i] = jb->customPhysicsTranslation * glm::translate(vertexPos) * jb->customPhysicsRotation * glm::translate(-vertexPos) * animationMatrix[i];
			}
		}

		return;
	}

	animationSystem->GetAnimationData(t, animationMatrix);
}

std::vector<glm::mat4> Model::GetAnimationData()
{
	return animationMatrix;
}

void Model::GetUnitBoneData(std::vector<glm::mat4>& data)
{
	animationSystem->GetToBoneFromUnit(data);
}

std::string Model::GetAnimationName()
{
	return animationSystem->GetAnimationName();
}

float Model::GetAnimationDuration()
{
	return animationSystem->GetAnimationDuration();
}

glm::vec3 Model::GetModelCentroid()
{
	return (boundingBox[0] + boundingBox[1]) * 0.5f;
}

void Model::GetToModelFromBone(std::vector<glm::mat4>& data)
{
	animationSystem->GetToModelFromBone(data);
}

void Model::AddBone(Bone* newBone)
{
	animationSystem->AddBone(newBone);

	// Add skeleton bone vertex buffer
	bones.push_back(newBone->toBoneFromUnit * glm::vec4(0.f, 0.f, 0.f, 1.f));
	bones.push_back(animationSystem->GetBone(newBone->parentID)->toBoneFromUnit * glm::vec4(0.f, 0.f, 0.f, 1.f));
}

void Model::ReadMaterial(const aiScene* scene, const std::string& path)
{
	//std::string::size_type slashIndex = path.find_last_of('/');
	//std::string dir;

	//if (slashIndex == std::string::npos)
	//{
	//	dir = ".";
	//}
	//else if (slashIndex == 0)
	//{
	//	dir = "/";
	//}
	//else
	//{
	//	dir = path.substr(0, slashIndex);
	//}

	//diffuseImagePaths.resize(scene->mNumMaterials);
	//normalImagePaths.resize(scene->mNumMaterials);

	//for (unsigned int i = 0; i < scene->mNumMaterials; i++)
	//{
	//	const aiMaterial* pMaterial = scene->mMaterials[i];

	//	if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
	//	{
	//		aiString pathToTexture;

	//		if (pMaterial->GetTexture(aiTextureType_DIFFUSE, i, &pathToTexture) == AI_SUCCESS)
	//		{
	//			std::string p(pathToTexture.data);

	//			if (p.substr(0, 2) == ".\\")
	//			{
	//				p = p.substr(2, p.size() - 2);
	//			}

	//			diffuseImagePaths[i] = dir + "/" + p;
	//		}
	//	}
	//	
	//	if (pMaterial->GetTextureCount(aiTextureType_HEIGHT) > 0)
	//	{
	//		aiString pathToTexture;

	//		if (pMaterial->GetTexture(aiTextureType_HEIGHT, i, &pathToTexture) == AI_SUCCESS)
	//		{
	//			std::string p(pathToTexture.data);

	//			if (p.substr(0, 2) == ".\\")
	//			{
	//				p = p.substr(2, p.size() - 2);
	//			}

	//			normalImagePaths[i] = dir + "/" + p;
	//		}
	//	}
	//}
}
