// Basic Texture shader and can also be used for colors if u_Texture is set to white.

#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TextureCoord;
layout(location = 3) in float a_TextureIndex;
layout(location = 4) in vec2 a_TilingFactor;
layout(location = 5) in int a_EntityID;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

struct VertexOutput
{
	vec4 Color;
	vec2 TextureCoord;
	float TextureIndex;
	vec2 TilingFactor;
};

layout (location = 0) out VertexOutput Output;
layout (location = 4) out flat int v_EntityID;

void main()
{
	Output.Color = a_Color;
	Output.TextureCoord = a_TextureCoord;
	Output.TextureIndex = a_TextureIndex;
	Output.TilingFactor = a_TilingFactor;
	v_EntityID = a_EntityID;

	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 color;
layout(location = 1) out int color2; // -1 is not an entity

struct	VertexOutput
{
	vec4 Color;
	vec2 TextureCoord;
	float TextureIndex;
	vec2 TilingFactor;
};

layout (location = 0) in VertexOutput Input;
layout (location = 4) in flat int v_EntityID;

layout (binding = 0) uniform sampler2D u_Texture[32];
//layout (binding = 0) uniform bool u_IsGrayscale;

void main()
{
	color = Input.Color;
	switch (int(Input.TextureIndex))
	{
		case  0: color *= texture(u_Texture[ 0], Input.TextureCoord * Input.TilingFactor); break;
		case  1: color *= texture(u_Texture[ 1], Input.TextureCoord * Input.TilingFactor); break;
		case  2: color *= texture(u_Texture[ 2], Input.TextureCoord * Input.TilingFactor); break;
		case  3: color *= texture(u_Texture[ 3], Input.TextureCoord * Input.TilingFactor); break;
		case  4: color *= texture(u_Texture[ 4], Input.TextureCoord * Input.TilingFactor); break;
		case  5: color *= texture(u_Texture[ 5], Input.TextureCoord * Input.TilingFactor); break;
		case  6: color *= texture(u_Texture[ 6], Input.TextureCoord * Input.TilingFactor); break;
		case  7: color *= texture(u_Texture[ 7], Input.TextureCoord * Input.TilingFactor); break;
		case  8: color *= texture(u_Texture[ 8], Input.TextureCoord * Input.TilingFactor); break;
		case  9: color *= texture(u_Texture[ 9], Input.TextureCoord * Input.TilingFactor); break;
		case 10: color *= texture(u_Texture[10], Input.TextureCoord * Input.TilingFactor); break;
		case 11: color *= texture(u_Texture[11], Input.TextureCoord * Input.TilingFactor); break;
		case 12: color *= texture(u_Texture[12], Input.TextureCoord * Input.TilingFactor); break;
		case 13: color *= texture(u_Texture[13], Input.TextureCoord * Input.TilingFactor); break;
		case 14: color *= texture(u_Texture[14], Input.TextureCoord * Input.TilingFactor); break;
		case 15: color *= texture(u_Texture[15], Input.TextureCoord * Input.TilingFactor); break;
		case 16: color *= texture(u_Texture[16], Input.TextureCoord * Input.TilingFactor); break;
		case 17: color *= texture(u_Texture[17], Input.TextureCoord * Input.TilingFactor); break;
		case 18: color *= texture(u_Texture[18], Input.TextureCoord * Input.TilingFactor); break;
		case 19: color *= texture(u_Texture[19], Input.TextureCoord * Input.TilingFactor); break;
		case 20: color *= texture(u_Texture[20], Input.TextureCoord * Input.TilingFactor); break;
		case 21: color *= texture(u_Texture[21], Input.TextureCoord * Input.TilingFactor); break;
		case 22: color *= texture(u_Texture[22], Input.TextureCoord * Input.TilingFactor); break;
		case 23: color *= texture(u_Texture[23], Input.TextureCoord * Input.TilingFactor); break;
		case 24: color *= texture(u_Texture[24], Input.TextureCoord * Input.TilingFactor); break;
		case 25: color *= texture(u_Texture[25], Input.TextureCoord * Input.TilingFactor); break;
		case 26: color *= texture(u_Texture[26], Input.TextureCoord * Input.TilingFactor); break;
		case 27: color *= texture(u_Texture[27], Input.TextureCoord * Input.TilingFactor); break;
		case 28: color *= texture(u_Texture[28], Input.TextureCoord * Input.TilingFactor); break;
		case 29: color *= texture(u_Texture[29], Input.TextureCoord * Input.TilingFactor); break;
		case 30: color *= texture(u_Texture[30], Input.TextureCoord * Input.TilingFactor); break;
		case 31: color *= texture(u_Texture[31], Input.TextureCoord * Input.TilingFactor); break;
	}
	
	//if(u_IsGrayscale)
	//{ 
	//	float grayScalevalue = (0.299 * color.x) + (0.587 * color.y) + (0.114 * color.z);
	//	color.x = grayScalevalue;
	//	color.y = grayScalevalue;
	//	color.z = grayScalevalue;
	//}
	
	//Testing
	color2 = v_EntityID; // placeholder for our entity ID
}
