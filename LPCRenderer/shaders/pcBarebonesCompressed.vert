#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 size;
layout(location = 2) in uint bufferOffset;
layout(location = 3) in uint bufferLength;

out VS_OUT
{
	vec3 size;
	uint bufferOffset;
	uint bufferLength;
} vs_out;

void main()
{
	vs_out.size = size;
	vs_out.bufferOffset = bufferOffset;
	vs_out.bufferLength = bufferLength;
	gl_Position = vec4(position, 1);	
}