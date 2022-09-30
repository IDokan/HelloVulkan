// a sample vertex shader to test compile and run shader

#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(binding = 0) uniform UniformBufferObject
{
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

layout(location = 0) out vec3 normal;
layout(location = 1) out vec3 viewVector;

void main()
{
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
	normal = normalize(vec3(transpose(inverse(ubo.model)) * vec4(inNormal, 1.f)));

	vec3 fragPos = vec3(ubo.model * vec4(inPosition, 1.f));

	viewVector = normalize(vec3(2, 2, 2) - fragPos);
}