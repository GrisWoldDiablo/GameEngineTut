// Basic Texture shader and can also be used for colors if u_Texture is set to white.

#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TextureCoord;
layout(location = 3) in int a_TextureIndex;
layout(location = 4) in vec2 a_TilingFactor;
layout(location = 5) in int a_EntityID;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
	vec2 u_Resolution;
};

struct VertexOutput
{
	vec4 Color;
	vec2 TextureCoord;
	vec2 TilingFactor;
	vec2 Resolution;
};

layout (location = 0) out VertexOutput Output;
layout (location = 4) out flat int v_EntityID;
layout (location = 5) out flat int v_TextureIndex;

void main()
{
	Output.Color = a_Color;
	Output.TextureCoord = a_TextureCoord;
	Output.TilingFactor = a_TilingFactor;
	Output.Resolution = u_Resolution;
	v_TextureIndex = a_TextureIndex;
	v_EntityID = a_EntityID;

	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 color;
layout(location = 1) out int entityID; // -1 is not an entity

struct	VertexOutput
{
	vec4 Color;
	vec2 TextureCoord;
	vec2 TilingFactor;
	vec2 Resolution;
};

layout (location = 0) in VertexOutput Input;
layout (location = 4) in flat int v_EntityID;
layout (location = 5) in flat int v_TextureIndex;

layout (binding = 0) uniform sampler2D u_Texture[32];

#define GRAYSCALE 0

void main()
{
	vec4 tempColor = Input.Color;
	if (v_TextureIndex >= 0 && v_TextureIndex < 32)
	{
		tempColor *= texture(u_Texture[v_TextureIndex], Input.TextureCoord * Input.TilingFactor);
	}

#if 0
	switch (v_TextureIndex)
	{
		case  0: tempColor *= texture(u_Texture[ 0], Input.TextureCoord * Input.TilingFactor); break;
		case  1: tempColor *= texture(u_Texture[ 1], Input.TextureCoord * Input.TilingFactor); break;
		case  2: tempColor *= texture(u_Texture[ 2], Input.TextureCoord * Input.TilingFactor); break;
		case  3: tempColor *= texture(u_Texture[ 3], Input.TextureCoord * Input.TilingFactor); break;
		case  4: tempColor *= texture(u_Texture[ 4], Input.TextureCoord * Input.TilingFactor); break;
		case  5: tempColor *= texture(u_Texture[ 5], Input.TextureCoord * Input.TilingFactor); break;
		case  6: tempColor *= texture(u_Texture[ 6], Input.TextureCoord * Input.TilingFactor); break;
		case  7: tempColor *= texture(u_Texture[ 7], Input.TextureCoord * Input.TilingFactor); break;
		case  8: tempColor *= texture(u_Texture[ 8], Input.TextureCoord * Input.TilingFactor); break;
		case  9: tempColor *= texture(u_Texture[ 9], Input.TextureCoord * Input.TilingFactor); break;
		case 10: tempColor *= texture(u_Texture[10], Input.TextureCoord * Input.TilingFactor); break;
		case 11: tempColor *= texture(u_Texture[11], Input.TextureCoord * Input.TilingFactor); break;
		case 12: tempColor *= texture(u_Texture[12], Input.TextureCoord * Input.TilingFactor); break;
		case 13: tempColor *= texture(u_Texture[13], Input.TextureCoord * Input.TilingFactor); break;
		case 14: tempColor *= texture(u_Texture[14], Input.TextureCoord * Input.TilingFactor); break;
		case 15: tempColor *= texture(u_Texture[15], Input.TextureCoord * Input.TilingFactor); break;
		case 16: tempColor *= texture(u_Texture[16], Input.TextureCoord * Input.TilingFactor); break;
		case 17: tempColor *= texture(u_Texture[17], Input.TextureCoord * Input.TilingFactor); break;
		case 18: tempColor *= texture(u_Texture[18], Input.TextureCoord * Input.TilingFactor); break;
		case 19: tempColor *= texture(u_Texture[19], Input.TextureCoord * Input.TilingFactor); break;
		case 20: tempColor *= texture(u_Texture[20], Input.TextureCoord * Input.TilingFactor); break;
		case 21: tempColor *= texture(u_Texture[21], Input.TextureCoord * Input.TilingFactor); break;
		case 22: tempColor *= texture(u_Texture[22], Input.TextureCoord * Input.TilingFactor); break;
		case 23: tempColor *= texture(u_Texture[23], Input.TextureCoord * Input.TilingFactor); break;
		case 24: tempColor *= texture(u_Texture[24], Input.TextureCoord * Input.TilingFactor); break;
		case 25: tempColor *= texture(u_Texture[25], Input.TextureCoord * Input.TilingFactor); break;
		case 26: tempColor *= texture(u_Texture[26], Input.TextureCoord * Input.TilingFactor); break;
		case 27: tempColor *= texture(u_Texture[27], Input.TextureCoord * Input.TilingFactor); break;
		case 28: tempColor *= texture(u_Texture[28], Input.TextureCoord * Input.TilingFactor); break;
		case 29: tempColor *= texture(u_Texture[29], Input.TextureCoord * Input.TilingFactor); break;
		case 30: tempColor *= texture(u_Texture[30], Input.TextureCoord * Input.TilingFactor); break;
		case 31: tempColor *= texture(u_Texture[31], Input.TextureCoord * Input.TilingFactor); break;
	}
#endif
	
#if GRAYSCALE
	float grayScalevalue = (0.299 * tempColor.x) + (0.587 * tempColor.y) + (0.114 * tempColor.z);
	tempColor = vec4(grayScalevalue, grayScalevalue, grayScalevalue, 1.0);
#endif

	// Final assignment
	color = tempColor;
	entityID = v_EntityID;
}
