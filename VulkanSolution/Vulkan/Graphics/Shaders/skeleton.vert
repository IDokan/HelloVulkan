
#version 450

layout(location = 0) in vec3 inPosition;

layout(binding = 0) uniform UniformBufferObject
{
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

struct AnimationData
{
	mat4 model;
};

layout(binding = 1) uniform AnimationBufferObject
{
	AnimationData item[99];
} data;

layout(push_constant) uniform constants
{
	int selectedBone;
} PushConstants;

layout(location = 0) out vec4 color;

void main()
{
	int id = (gl_VertexIndex + gl_InstanceIndex) / 2;
	if(PushConstants.selectedBone == id)
	{
		color = vec4(1.f, 0.f, 1.f, 1.f);
	}
	else
	{
		color = vec4(1.f, 1.f, 1.f, 1.f);
	}
	gl_Position = ubo.proj * ubo.view * ubo.model * data.item[id].model * vec4(inPosition, 1.0);
}