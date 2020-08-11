// Vertex Color shader

#type vertex
#version 430
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec3 v_Position;
out vec4 v_Color;

void main()
{
	v_Color = a_Color;
	v_Position = a_Position;
	gl_Position = u_ViewProjection * u_Transform * vec4(v_Position, 1.0);
}

#type fragment
#version 430
layout(location = 0) out vec4 color;

uniform float u_Time;

in vec3 v_Position;
in vec4 v_Color;

void main()
{
	float t = (sin(u_Time) + 1) * 0.5;
	float tt = (cos(u_Time) + 1) * 0.5;
	color = vec4(1);
	float x = v_Color.x;
	float y = v_Color.y;
	float b = 0.1;
	for(int i = 0 ; i < 5 ; i++)
	{
		//t*=0.01*i;
		//tt*=0.01*i;
		b *= i;
		float f = smoothstep(t,t+0.1,x+b) - smoothstep(t+0.2,t+0.25,x+b);
		float ff = smoothstep(t,t+0.1,y+b) - smoothstep(t+0.2,t+0.25,y+b);
		color *= vec4(v_Color.xyz * f * ff, v_Color.w * ff * f);
	}
	//color = vec4(v_Color.x *t,v_Color.y*tt*f,v_Color.z*t, v_Color.w) ;
}
