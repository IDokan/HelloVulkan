// a sample fragment shader to test compile and run shader

#version 450

layout(location = 0) out vec4 outColor;


flat layout(location = 0) in uint i;

void main()
{
	outColor = vec4(1.0, 0.0, 0.0, 1.0); 
	if(i == 1)
	{
		outColor.g = 1.0; 
	}
	if(i == 1)
	{
		outColor.b = 1.0;
	}
}