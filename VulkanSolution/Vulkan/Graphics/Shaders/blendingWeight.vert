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

layout(push_constant) uniform constants
{
	int selectedBone;
} PushConstants;

layout(location = 0) out vec3 blendingColor;

vec3 GetBlendingColor(float weight)
{
	if(weight <= 0.5f)
	{
		weight *= 2.f;
		return (1.0f - weight) * vec3(0.f, 0.f, 1.f) + (weight)*vec3(0.f, 1.f, 0.f);
	}
	else
	{
		weight = (weight - 0.5f) * 2.f;
		return (1.0f - weight) * vec3(0.f, 1.f, 0.f) + (weight)*vec3(1.f, 0.f, 0.f);
	}
}

void main()
{
	mat4 animationTransform = data.item[boneIDs[0]].model * boneWeights[0];
	animationTransform += data.item[boneIDs[1]].model * boneWeights[1];
	animationTransform += data.item[boneIDs[2]].model * boneWeights[2];
	animationTransform += data.item[boneIDs[3]].model * boneWeights[3];

	float weight = 0.f;
	for(int i = 0; i < 4; i++)
	{
		if(boneIDs[i] == PushConstants.selectedBone)
		{
			weight += boneWeights[i];
		}
	}
	blendingColor = GetBlendingColor(weight);
	

	gl_Position = ubo.proj * ubo.view * ubo.model * animationTransform * vec4(inPosition, 1.0);
}