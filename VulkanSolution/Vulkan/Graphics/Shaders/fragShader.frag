// a sample fragment shader to test compile and run shader

#version 450

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 normal;
layout(location = 1) in vec3 viewVector;
layout(location = 2) in vec2 fragTexCoord;

layout(binding = 1) uniform sampler2D texSampler;

void main()
{
	vec3 color = vec3(1.f, 1.f, 1.f) * dot(normal, viewVector);
	outColor = vec4(color, 1.f) * texture(texSampler, fragTexCoord);
	// outColor = vec4(color, 1.f) * vec4(fragTexCoord, 0.f, 1.f);
}