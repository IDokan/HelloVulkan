// a sample vertex shader to test compile and run shader

#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inVertexColor;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in ivec4 boneIDs;
layout(location = 5) in vec4 boneWeights;

layout(binding = 0) uniform UniformBufferObject
{
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

struct AnimationData
{
	mat4 model;
};

layout(binding = 1) uniform AnimationBufferObject
{
	AnimationData item[99];
} data;

layout(location = 0) out vec3 normal;
layout(location = 1) out vec3 viewVector;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out vec3 vertexColor;

layout(push_constant) uniform constants
{
	float pointSize;
	int vertexID;
	bool isMousePressed;
} PushConstants;

void main()
{
	gl_PointSize = PushConstants.pointSize;

	mat4 animationTransform = data.item[boneIDs[0]].model * boneWeights[0];
	animationTransform += data.item[boneIDs[1]].model * boneWeights[1];
	animationTransform += data.item[boneIDs[2]].model * boneWeights[2];
	animationTransform += data.item[boneIDs[3]].model * boneWeights[3];

	gl_Position = ubo.proj * ubo.view * ubo.model * animationTransform * vec4(inPosition, 1.0);
	normal = normalize(vec3(transpose(inverse(ubo.model)) * animationTransform * vec4(inNormal, 0.f)));

	vec3 fragPos = vec3(ubo.model * animationTransform * vec4(inPosition, 1.f));

	viewVector = normalize(vec3(0, 0, 2) - fragPos);
	
	vertexColor = vec3(1, 0, 0);
	if(gl_VertexIndex == PushConstants.vertexID)
	{
		if(PushConstants.isMousePressed)
		{
			vertexColor = vec3(0, 1, 0);
		}
		else
		{
			vertexColor = vec3(1, 1, 1);
		}
	}
	
}