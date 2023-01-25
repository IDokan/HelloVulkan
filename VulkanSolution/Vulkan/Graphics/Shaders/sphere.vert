// a sample vertex shader to test compile and run shader

#version 450

layout(location = 0) in vec3 inPosition;

layout(binding = 0) uniform UniformBufferObject
{
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

layout(push_constant) uniform constants
{
	mat4 sphereBoundingMatrix;
	mat4 translation;
	float radius;
} PushConstants;



void main()
{
	gl_Position = ubo.proj * ubo.view * ubo.model * PushConstants.translation * PushConstants.sphereBoundingMatrix * vec4(inPosition * PushConstants.radius, 1.0);
}