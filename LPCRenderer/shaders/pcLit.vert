#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform bool backFaceCulling;

out gl_PerVertex
{
  vec4 gl_Position;
  float gl_CullDistance[1];
};

out VS_OUT
{
	vec3 position;
	vec3 normal;
} vs_out;

void main()
{
	vs_out.position = vec3(view * model * vec4(position, 1.0f));
	vs_out.normal = mat3(transpose(inverse(view * model))) * normal;
	if(backFaceCulling && dot(vs_out.normal, -vs_out.position) < 0)
		gl_CullDistance[0] = -1.0f;
	else
		gl_CullDistance[0] = 1.0f;
	gl_Position = projection * view * model * vec4(position, 1.0f);	
}