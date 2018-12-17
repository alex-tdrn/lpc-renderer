#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in mat4 t;

uniform mat4 mvp;

out float occupancy;

void main()
{
	occupancy = t[0][3];
	mat4 transform = t;
	transform[0][3] = 0.0f;
	gl_Position = mvp * transform * vec4(position, 1);	
}