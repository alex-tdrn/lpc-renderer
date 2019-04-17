#version 450 core

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

	gl_Position = projection * view * model * p;
	gs_out.color = vec3(0.0f, 0.0f, 0.0f);
	EmitVertex();

	gl_Position = projection * view * model * (p + vec4(gs_in[0].normal * lineLength, 0.0f));
	gs_out.color = gs_in[0].normal;
	EmitVertex();

	EndPrimitive();
}