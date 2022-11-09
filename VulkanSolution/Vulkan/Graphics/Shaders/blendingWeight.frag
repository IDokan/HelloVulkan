// a sample fragment shader to test compile and run shader

#version 450

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 blendingColor;

void main()
{
	outColor = vec4(blendingColor, 1.f);
}