#version 460 core

uniform vec3 cloudOrigin;
uniform vec3 brickSize;
layout(location = 0) in uvec3 brickIndex;
layout(location = 1) in uint bufferOffset;
layout(location = 2) in uint bufferLength;

out VS_OUT
{
	uint bufferOffset;
	uint bufferLength;
} vs_out;

void main()
{
	vs_out.bufferOffset = bufferOffset;
	vs_out.bufferLength = bufferLength;

	gl_Position = vec4(cloudOrigin + brickIndex * brickSize, 1);	

}