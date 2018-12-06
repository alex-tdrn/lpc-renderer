#version 420 core

layout(location = 0) in vec3 position;
layout(location = 1) in mat4 t;

uniform mat4 mvp;

out vec3 color;

void main()
{
	color = vec3(1.0f, 0.0f, 0.0f);
	mat4 transform = t;
	if(t[0][3] > 0.5f)
	{
		color = vec3(1.0f);
		transform[0][3] = 0.0f;
	}
	gl_Position = mvp * transform * vec4(position, 1);	
}