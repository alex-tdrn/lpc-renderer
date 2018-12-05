#version 420 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform bool backFaceCulling;
uniform float diskRadius;

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in VS_OUT
{
	vec3 viewSpacePosition;
	vec3 viewSpaceNormal;
	vec3 modelSpaceNormal;
} gs_in[];

out GS_OUT
{
	vec3 position;
	vec3 normal;
	vec2 uv;
} gs_out;

vec3 x;
vec3 y;

void pushVertex(float u, float v)
{
	gs_out.normal = gs_in[0].viewSpaceNormal;
	gs_out.position = gs_in[0].viewSpacePosition;
	gs_out.uv = vec2(u, v);
	vec3 offset = x * u + y * v;
	gl_Position = projection * view * model * vec4(gl_in[0].gl_Position.xyz + offset, 1.0f);
	EmitVertex();
}

void main()
{
	if(backFaceCulling && dot(gs_in[0].viewSpaceNormal, -gs_in[0].viewSpacePosition) < 0)
		return;
	if(abs(dot(gs_in[0].modelSpaceNormal, vec3(1.0f, 0.0f, 0.0f))) > 0.01f)
		x = cross(gs_in[0].modelSpaceNormal, vec3(1.0f, 0.0f, 0.0f));
	else
		x = cross(gs_in[0].modelSpaceNormal, vec3(0.0f, 1.0f, 0.0f));
	y = cross(gs_in[0].modelSpaceNormal, x);
	x = normalize(x) * diskRadius;
	y = normalize(y) * diskRadius;
	
	pushVertex(-1.0f, +1.0f);
	pushVertex(-1.0f, -1.0f);
	pushVertex(+1.0f, +1.0f);
	pushVertex(+1.0f, -1.0f);
	EndPrimitive();
}