#version 420 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out VS_OUT
{
	vec3 viewSpacePosition;
	vec3 viewSpaceNormal;
	vec3 modelSpaceNormal;
} vs_out;

void main()
{
	vs_out.viewSpacePosition = vec3(view * model * vec4(position, 1.0f));
	vs_out.viewSpaceNormal = mat3(transpose(inverse(view * model))) * normal;

	vs_out.modelSpaceNormal = normal;
	gl_Position = vec4(position, 1.0f);	
}