#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in mat4 t;

uniform mat4 mvp;

void main()
{
	gl_Position = mvp * t * vec4(position, 1);	
}