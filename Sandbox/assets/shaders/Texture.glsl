// Basic Texture shader and cal also be used for colors if u_Texture is set to white.

#type vertex
#version 430

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec2 v_TexCoord;

void main()
{
	v_TexCoord = a_TexCoord;
	gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 430

layout(location = 0) out vec4 color;

uniform sampler2D u_Texture;
uniform vec4 u_Color;
uniform vec2 u_TilingFactor;

in vec2 v_TexCoord;

void main()
{
	color = u_Color;
	color *= texture(u_Texture, v_TexCoord * u_TilingFactor);
}
