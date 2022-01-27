#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in int a_EntityID;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

struct VertexOutput
{
	vec4 Color;
};

layout (location = 0) out VertexOutput Output;
layout (location = 1) out flat int v_EntityID;

void main()
{
	Output.Color = a_Color;
	v_EntityID = a_EntityID;

	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID; // -1 is not an entity

struct	VertexOutput
{
	vec4 Color;
};

layout (location = 0) in VertexOutput Input;
layout (location = 1) in flat int v_EntityID;

#define GRAYSCALE 0

void main()
{
	vec4 tempColor = Input.Color;
	
	
#if GRAYSCALE
	float grayScalevalue = (0.299 * tempColor.x) + (0.587 * tempColor.y) + (0.114 * tempColor.z);
	tempColor = vec4(grayScalevalue, grayScalevalue, grayScalevalue, 1.0);
#endif

	if (tempColor.a < 0.01)
	{
		discard;
	}

	// Final assignment
	o_Color = tempColor;
	o_EntityID = v_EntityID;
}
