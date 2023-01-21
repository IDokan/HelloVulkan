// a sample vertex shader to test compile and run shader

#version 450

layout(location = 0) in vec3 inPosition;

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
} animData;

layout(binding = 2) uniform UnitBoneObject
{
	AnimationData item[99];
} unitData;

layout(binding = 3) uniform HairBoneBufferObject
{
	vec3 offset[90];
} data;

layout(push_constant) uniform constants
{
	float pointSize;
	int selectedBone;
} PushConstants;



void main()
{
	gl_PointSize = PushConstants.pointSize;
	gl_Position = ubo.proj * ubo.view * ubo.model * unitData.item[PushConstants.selectedBone].model * vec4(data.offset[gl_InstanceIndex], 1.0);
}