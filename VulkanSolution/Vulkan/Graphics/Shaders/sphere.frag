// a sample fragment shader to test compile and run shader

#version 450

layout(location = 0) out vec4 outColor;

void main()
{
	outColor = vec4(1.f, 1.f, 0.f, 1.f);
}