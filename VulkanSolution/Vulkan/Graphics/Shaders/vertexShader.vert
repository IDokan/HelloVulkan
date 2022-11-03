// a shader to display a static model.

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

layout(location = 0) out vec3 normal;
layout(location = 1) out vec3 viewVector;
layout(location = 2) out vec2 fragTexCoord;

void main()
{
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
	normal = normalize(vec3(transpose(inverse(ubo.model)) * vec4(inNormal, 0.f)));
	fragTexCoord = inTexCoord;

	vec3 fragPos = vec3(ubo.model * vec4(inPosition, 1.f));

	viewVector = normalize(vec3(2, 2, 2) - fragPos);
}