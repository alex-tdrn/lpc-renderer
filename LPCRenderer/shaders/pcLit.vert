#version 420 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out VS_OUT
{
	vec3 position;
	vec3 normal;
} vs_out;

void main()
{
	vs_out.position = vec3(view * model * vec4(position, 1.0f));
	vs_out.normal = mat3(transpose(inverse(view * model))) * (normal);
	gl_Position = projection * view * model * vec4(position, 1.0f);	
}