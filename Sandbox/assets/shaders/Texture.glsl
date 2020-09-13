// Basic	 Texture shader and cal also be used for colors if u_Texture is set to white.

#type vertex
#version 430

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TextureCoord;
layout(location = 3) in float a_TextureIndex;
layout(location = 4) in vec2 a_TilingFactor;

out vec4 v_Color;
out vec2 v_TextureCoord;
out float v_TextureIndex;
out vec2 v_TilingFactor;

uniform mat4 u_ViewProjection;

void main()
{
	v_Color = a_Color;
	v_TextureCoord = a_TextureCoord;
	v_TextureIndex = a_TextureIndex;
	v_TilingFactor = a_TilingFactor;
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 430

layout(location = 0) out vec4 color;

in vec4 v_Color;
in vec2 v_TextureCoord;
in float v_TextureIndex;
in vec2 v_TilingFactor;

uniform sampler2D u_Texture[32];

void main()
{
	
	color = v_Color;
	color *= texture(u_Texture[int(v_TextureIndex)], v_TextureCoord * v_TilingFactor);
}
