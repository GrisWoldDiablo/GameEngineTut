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
	vec3 u_Position;
	//vec2 u_Resolution;
};

struct VertexOutput 
{
	vec4 Color;
	vec2 TextureCoord;
	vec2 TilingFactor;
	vec3 CameraPosition;
	//vec2 Resolution;
};

layout(location = 0) out VertexOutput Output;
//layout(location = 5) out flat int v_EntityID;
//layout(location = 6) out vec3 v_Position;

void main() 
{
	Output.Color = a_Color;
	Output.TextureCoord = a_TextureCoord;
	Output.TilingFactor = a_TilingFactor;
	Output.CameraPosition = u_Position;
	//Output.Resolution = vec2(0);//u_Resolution;

	//v_EntityID = a_EntityID;
	//v_Position = a_Position;

	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 color;
layout(location = 1) out int entityID; // -1 is not an entity

struct VertexOutput 
{
	vec4 Color;
	vec2 TextureCoord;
	vec2 TilingFactor;
	vec3 CameraPosition;
	//vec2 Resolution;
};

layout(location = 0) in VertexOutput Input;
//layout(location = 5) in flat int v_EntityID;
//layout(location = 6) in vec3 v_Position;

float IntersectSphere(vec3 ro, vec3 rd,vec4 sphere)
{
    float r = sphere.w;
    float t = dot(sphere.xyz-ro,rd);
    vec3 p = ro+rd * t;
   	float y = length(sphere.xyz-p);
    if(y<r){
    	float x = sqrt(r*r - y*y);
    	float t1 = t-x;
    	float t2 = t+x;
        return t1;
    }
}

void main() 
{
	vec4 tempColor = Input.Color;
	vec2 tiling = Input.TilingFactor;
	tiling.x = 1920.0;
	tiling.y = 1080.0;
	vec2 coord = Input.TextureCoord;
	coord.x += clamp(Input.CameraPosition.x, -0.5, 0.5);
	coord.y += clamp(Input.CameraPosition.y, -0.5, 0.5);
	vec2 p = (coord - 0.5) * tiling /  tiling.y;
	//uv.x *= Input.Resolution.x / Input.Resolution.y;
    //vec3 col = vec3(0);
	//float zoom = 1.;O
    //vec3 ro = v_CameraPosition;
    //vec3 f = normalize(v_Position-ro);
    //vec3 lookat = v_Position + f * -10;
    //vec3 rg = normalize(cross(vec3(0,1,0),f));
    //vec3 u = normalize(cross(f,rg));
    //vec3 c = ro + f*zoom;
    //vec3 i  = c + uv.x*rg + uv.y*u;
    //vec3 rd = normalize(i-ro);
    //col += IntersectSphere(ro, rd,vec4(0,0,5,1));
	
	// Final assignment
	//color = vec4(result, 1.0);
	//if (uv.x < 0 && uv.y < 0)
	//{
	//	discard;
	//}
	//
	//float d = length(uv);
	//float r = Input.CameraPosition.x;
    //
    //float c = smoothstep(1-r, r, d);
    
    //if(d < .3) c = 1.; else c = 0.;
    
    //color = vec4(vec3(c), 1.0);

	float r2 = dot(p, p);
	float r = sqrt(r2);
	vec2 uv = p/r2;

	uv += vec2(0.0, 2.0 * cos(uv.y * 8.0));
	uv = abs(sin(uv* 0.3));

	float colo = smoothstep(0.2, 0.8, abs(sin(uv.y * 3.0)));
	colo = min(colo, smoothstep(0.1, 0.95, abs(sin(uv.y * 4.0))));
	colo += 0.75;
	vec3 col = vec3(0.6 + 0.1 * cos(colo), 0.5 * colo, 0.9 + 0.2 * sin(colo * 1.1));

	col *= r * 1.5 * colo;
	col += pow(length(p) / 2.0, 2.0);

	color = vec4(col, 1.0);
	entityID = -1;
}
