// a sample vertex shader to test compile and run shader

#version 450

vec2 positions[3] = vec2[](
	vec2(1.0, -1.0),
	vec2(-1.0, -1.0),
	vec2(-1.0, 1.0)
);



vec4 colors[3] = vec4[](
	vec4(1.0, 1.0, 0.0, 1.0),
	vec4(1.0, 0.0, 0.0, 1.0),
	vec4(0.0, 1.0, 0.0, 1.0)
);

layout(location = 0) out vec4 fragColor;

void main()
{
	gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);

	fragColor = colors[gl_VertexIndex];
}