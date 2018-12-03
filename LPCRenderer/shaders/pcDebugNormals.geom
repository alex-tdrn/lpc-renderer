#version 420 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 diffuseColor;
uniform float lineLength;

layout(points) in;
layout(line_strip, max_vertices = 2) out;

in VS_OUT
{
	vec3 normal;
} gs_in[];

out GS_OUT
{
	vec3 color;
} gs_out;

void main()
{
	vec4 p = gl_in[0].gl_Position;
	vec3 n = gs_in[0].normal;

	n = mat3(transpose(inverse(model))) * n;
	gl_Position = projection * view * model * p;
	gs_out.color = diffuseColor;
	EmitVertex();
	gl_Position = projection * view * model * (p + vec4(n * lineLength, 0.0f));
	gs_out.color = 1.0f - diffuseColor;
	EmitVertex();
	EndPrimitive();
}