// Basic	 Texture shader and cal also be used for colors if u_Texture is set to white.

#type vertex
#version 430

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in vec2 a_TilingFactor;

out vec4 v_Color;
out vec2 v_TexCoord;
out float v_TexIndex;
out vec2 v_TilingFactor;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

void main()
{
	v_Color = a_Color;
	v_TexCoord = a_TexCoord;
	v_TexIndex = a_TexIndex;
	v_TilingFactor = a_TilingFactor;
	gl_Position = u_ViewProjection * /*u_Transform **/ vec4(a_Position, 1.0);
}

#type fragment
#version 430

layout(location = 0) out vec4 color;

in vec4 v_Color;
in vec2 v_TexCoord;
in float v_TexIndex;
in vec2 v_TilingFactor;

uniform sampler2D u_Texture[32];

void main()
{
	
	color = v_Color;
	color *= texture(u_Texture[int(v_TexIndex)], v_TexCoord * v_TilingFactor);
}
