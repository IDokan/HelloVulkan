// a sample fragment shader to test compile and run shader

#version 450

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 normal;
layout(location = 1) in vec3 viewVector;
layout(location = 3) in vec3 vertexColor;

void main()
{
	vec3 color = vertexColor * dot(normal, viewVector);
	outColor = vec4(vertexColor, 1.f);
}