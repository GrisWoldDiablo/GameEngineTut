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
uniform bool u_IsGrayscale;

void main()
{
	color = v_Color;
	switch (int(v_TextureIndex))
	{
		case 0: color *= texture(u_Texture[0], v_TextureCoord * v_TilingFactor); break;
		case 1: color *= texture(u_Texture[1], v_TextureCoord * v_TilingFactor); break;
		case 2: color *= texture(u_Texture[2], v_TextureCoord * v_TilingFactor); break;
		case 3: color *= texture(u_Texture[3], v_TextureCoord * v_TilingFactor); break;
		case 4: color *= texture(u_Texture[4], v_TextureCoord * v_TilingFactor); break;
		case 5: color *= texture(u_Texture[5], v_TextureCoord * v_TilingFactor); break;
		case 6: color *= texture(u_Texture[6], v_TextureCoord * v_TilingFactor); break;
		case 7: color *= texture(u_Texture[7], v_TextureCoord * v_TilingFactor); break;
		case 8: color *= texture(u_Texture[8], v_TextureCoord * v_TilingFactor); break;
		case 9: color *= texture(u_Texture[9], v_TextureCoord * v_TilingFactor); break;
		case 10: color *= texture(u_Texture[10], v_TextureCoord * v_TilingFactor); break;
		case 11: color *= texture(u_Texture[11], v_TextureCoord * v_TilingFactor); break;
		case 12: color *= texture(u_Texture[12], v_TextureCoord * v_TilingFactor); break;
		case 13: color *= texture(u_Texture[13], v_TextureCoord * v_TilingFactor); break;
		case 14: color *= texture(u_Texture[14], v_TextureCoord * v_TilingFactor); break;
		case 15: color *= texture(u_Texture[15], v_TextureCoord * v_TilingFactor); break;
		case 16: color *= texture(u_Texture[16], v_TextureCoord * v_TilingFactor); break;
		case 17: color *= texture(u_Texture[17], v_TextureCoord * v_TilingFactor); break;
		case 18: color *= texture(u_Texture[18], v_TextureCoord * v_TilingFactor); break;
		case 19: color *= texture(u_Texture[19], v_TextureCoord * v_TilingFactor); break;
		case 20: color *= texture(u_Texture[20], v_TextureCoord * v_TilingFactor); break;
		case 21: color *= texture(u_Texture[21], v_TextureCoord * v_TilingFactor); break;
		case 22: color *= texture(u_Texture[22], v_TextureCoord * v_TilingFactor); break;
		case 23: color *= texture(u_Texture[23], v_TextureCoord * v_TilingFactor); break;
		case 24: color *= texture(u_Texture[24], v_TextureCoord * v_TilingFactor); break;
		case 25: color *= texture(u_Texture[25], v_TextureCoord * v_TilingFactor); break;
		case 26: color *= texture(u_Texture[26], v_TextureCoord * v_TilingFactor); break;
		case 27: color *= texture(u_Texture[27], v_TextureCoord * v_TilingFactor); break;
		case 28: color *= texture(u_Texture[28], v_TextureCoord * v_TilingFactor); break;
		case 29: color *= texture(u_Texture[29], v_TextureCoord * v_TilingFactor); break;
		case 30: color *= texture(u_Texture[30], v_TextureCoord * v_TilingFactor); break;
		case 31: color *= texture(u_Texture[31], v_TextureCoord * v_TilingFactor); break;
	}

	if(u_IsGrayscale)
	{ 
		float grayScalevalue = (0.299 * color.x) + (0.587 * color.y) + (0.114 * color.z);
		color.x = grayScalevalue;
		color.y = grayScalevalue;
		color.z = grayScalevalue;
	}
}
