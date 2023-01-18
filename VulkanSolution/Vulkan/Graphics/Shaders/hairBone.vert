// a sample vertex shader to test compile and run shader

#version 450

layout(location = 0) in vec3 inPosition;

layout(binding = 0) uniform UniformBufferObject
{
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

layout(binding = 1) uniform HairBoneBufferObject
{
	vec3 offset[90];
} data;

layout(push_constant) uniform constants
{
	float pointSize;
} PushConstants;

void main()
{
	gl_PointSize = PushConstants.pointSize;
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(data.offset[gl_InstanceIndex], 1.0);
}