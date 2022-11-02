// a sample vertex shader to test compile and run shader

#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in ivec4 boneIDs;
layout(location = 4) in vec4 boneWeights;

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

layout(binding = 2) uniform AnimationBufferObject
{
	AnimationData item[99];
} data;

layout(location = 0) out vec3 normal;
layout(location = 1) out vec3 viewVector;
layout(location = 2) out vec2 fragTexCoord;

void main()
{
	mat4 animationTransform = data.item[boneIDs[0]].model * boneWeights[0];
	animationTransform += data.item[boneIDs[1]].model * boneWeights[1];
	animationTransform += data.item[boneIDs[2]].model * boneWeights[2];
	animationTransform += data.item[boneIDs[3]].model * boneWeights[3];

	gl_Position = ubo.proj * ubo.view * ubo.model * animationTransform * vec4(inPosition, 1.0);
	normal = normalize(vec3(transpose(inverse(ubo.model)) * animationTransform * vec4(inNormal, 0.f)));
	fragTexCoord = inTexCoord;

	vec3 fragPos = vec3(ubo.model * animationTransform * vec4(inPosition, 1.f));

	viewVector = normalize(vec3(2, 2, 2) - fragPos);
}