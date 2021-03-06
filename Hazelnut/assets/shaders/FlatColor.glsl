// Basic Flat Color shader

#type vertex
#version 430

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 5) in int a_EntityID;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

layout(location = 0) out vec4 v_Color;
layout(location = 1) out flat int v_EntityID;

void main()
{
	v_Color = a_Color;
	v_EntityID = a_EntityID;
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 430

layout(location = 0) out vec4 color;
layout(location = 1) out int color2;

layout(location = 0) in vec4 v_Color;
layout(location = 1) in flat int v_EntityID;

void main()
{
	color = v_Color;
	color2 = v_EntityID;
}
