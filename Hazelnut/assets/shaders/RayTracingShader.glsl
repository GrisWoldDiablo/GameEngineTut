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
	vec2 u_Resolution;
};

struct VertexOutput
{
	vec4 Color;
	vec2 TextureCoord;
	float TextureIndex;
	vec2 TilingFactor;
	vec2 Resolution;
};

layout (location = 0) out VertexOutput Output;
layout (location = 5) out flat int v_EntityID;
layout (location = 6) out vec3 v_Position;

void main()
{
	Output.Color = a_Color;
	Output.TextureCoord = a_TextureCoord;
	Output.TextureIndex = a_TextureIndex;
	Output.TilingFactor = a_TilingFactor;
	Output.Resolution = u_Resolution;
	v_EntityID = a_EntityID;
	v_Position = a_Position;

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
	vec2 Resolution;
};

layout (location = 0) in VertexOutput Input;
layout (location = 5) in flat int v_EntityID;
layout (location = 6) in vec3 v_Position;

layout (binding = 0) uniform sampler2D u_Texture[32];

#define GRAYSCALE 0

#define REFLECTION_AMOUNT 4
#define Inf 10000.

struct Sphere
{
	vec3 position;
	float radius;
	vec3 albedo;
	vec3 specular;
};
 
Sphere CreateSphere(vec4 it, vec3 alb, vec3 spec)
{
    Sphere sphere;
    sphere.position = it.xyz;
 	sphere.radius = it.w;
	sphere.albedo = alb;
	sphere.specular = spec;
	return sphere;
}

struct Ray
{
	vec3 origin;
    vec3 direction;
    vec3 energy;
};
    
Ray CreateRay(vec3 origin, vec3 direction)
{
    Ray ray;
    ray.origin = origin;
    ray.direction = direction;
    ray.energy = vec3(1);
    return ray;
}

struct RayHit
{
	vec3 position;
	float distance;
	vec3 normal;
	vec3 albedo;
	vec3 specular;
};

RayHit CreateRayHit()
{
	RayHit hit;
	hit.position = vec3(0);
	hit.distance = Inf;
	hit.normal = vec3(0);
	return hit;
}

void IntersectGroundPlane(Ray ray, inout RayHit bestHit)
{
    float t = -ray.origin.y / ray.direction.y;
    if(t > 0. && t < bestHit.distance)
    {
        bestHit.distance = t;
        bestHit.position = ray.origin + t * ray.direction;
        bestHit.normal = vec3(0,1,0);
        bestHit.albedo = vec3(.2);
        bestHit.specular = vec3(.1);
    }
}

void IntersectSphere(Ray ray, inout RayHit bestHit, Sphere sphere)
{
	// Calculate distance along the ray where the sphere is intersected
	vec3 d = ray.origin - sphere.position;
	float p1 = -dot(ray.direction, d);
	float p2sqr = p1 * p1 - dot(d, d) + sphere.radius * sphere.radius;
	if (p2sqr < 0.)
	{
		return;
	}

	float p2 = sqrt(p2sqr);
	float t = p1 - p2 > 0. ? p1 - p2 : p1 + p2;
	if (t > 0. && t < bestHit.distance)
	{
		bestHit.distance = t;
		bestHit.position = ray.origin + t * ray.direction;
		bestHit.normal = normalize(bestHit.position - sphere.position);
		bestHit.albedo = sphere.albedo;
		bestHit.specular = sphere.specular;
	}
}

Ray CreateCameraRay(vec2 uv, vec2 M)
{
    vec3 origin = vec3(20.*sin(M.x*5.),2.,-30.*cos(M.x*5.) - 20.);
    
    vec3 lookat = vec3(0,2.*M.y*5.+10.,20.);
    vec3 f = normalize(lookat-origin);
    vec3 rg = normalize(cross(vec3(0,1,0),f));
    vec3 u = normalize(cross(f,rg));
    vec3 c = origin + f*1.;
    vec3 i  = c + uv.x*rg + uv.y*u;

    vec3 direction = normalize(i-origin);
    return CreateRay(origin, direction);
}

RayHit Trace(Ray ray)
{
    RayHit bestHit = CreateRayHit();
    IntersectGroundPlane(ray, bestHit);
    
    Sphere sphere = CreateSphere(vec4(0,2.,5,1),vec3(1,0,0),vec3(0));
    IntersectSphere(ray, bestHit, sphere);
    Sphere sphere2 = CreateSphere(vec4(2,2,5.,.5),vec3(0,1,0),vec3(1));
    IntersectSphere(ray, bestHit, sphere2);
    Sphere sphere3 = CreateSphere(vec4(-2,2,5.,.5),vec3(0,1,1),vec3(1));
    IntersectSphere(ray, bestHit, sphere3);
    Sphere sphere4 = CreateSphere(vec4(0.,4.,5.,.5),vec3(1,1,0),vec3(.5));
    IntersectSphere(ray, bestHit, sphere4);
    Sphere sphere5 = CreateSphere(vec4(0.,15,20,10),vec3(1),vec3(1));
    IntersectSphere(ray, bestHit, sphere5);
    return bestHit;
}

vec3 Shade(inout Ray ray, RayHit hit)
{
	if(hit.distance < Inf)
    {
    	ray.origin = hit.position + hit.normal * 0.001;
        ray.direction = reflect(ray.direction, hit.normal);
        ray.energy *= hit.specular;
        
        vec3 lightPos = normalize(vec3(1.,2.,-5.));
        
        Ray shadowRay = CreateRay(hit.position + hit.normal * 0.001, lightPos);
		RayHit shadowHit = Trace(shadowRay);
        if (shadowHit.distance != Inf)
		{
			return vec3(0);
		}
        return clamp(dot(hit.normal, lightPos) * 1.,0.,1.)* hit.albedo;
    }
    else
    {
        ray.energy = vec3(0);
        return vec3(.2);
    }
    
}

void main()
{
	vec4 tempColor = Input.Color;

	//if(u_IsGrayscale)
	vec2 uv = (Input.TextureCoord - 0.5)* Input.Resolution.xy / Input.Resolution.y;// * Input.Resolution.xy)/;
	tempColor = vec4(uv, 0.0, 1.0);

    vec2 centerPosition = (vec2(Input.Resolution.x / 2.,Input.Resolution.y / 2.) / Input.Resolution.xy) * 2. -1.;
    centerPosition = (vec2((v_Position.x-200) * 400.,v_Position.y * 1000.) / Input.Resolution.xy) * 2. -1.;

	Ray ray = CreateCameraRay(uv, centerPosition);
    
    vec3 result = vec3(0);
    for (int i = 0; i < REFLECTION_AMOUNT; i++)
    {
    	RayHit hit = Trace(ray);
    	result += ray.energy * Shade(ray, hit);
        if(ray.energy.x == 0. || ray.energy.y == 0. || ray.energy.z == 0.) break;
    }

#if GRAYSCALE
	float grayScalevalue = (0.299 * result.x) + (0.587 * result.y) + (0.114 * result.z);
	result = vec3(grayScalevalue);
#endif

	// Final assignement
	color = vec4(result,1.0);
	color2 = v_EntityID;
}
