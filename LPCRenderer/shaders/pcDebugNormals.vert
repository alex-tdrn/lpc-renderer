#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

out VS_OUT
{
	vec3 normal;
} vs_out;

void main()
{
	vs_out.normal = normal;
	gl_Position = vec4(position, 1.0f);	
}