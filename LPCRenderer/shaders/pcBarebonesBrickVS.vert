#version 450 core

uniform vec3 cloudOrigin;
uniform vec3 brickSize;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

layout(location = 0) in uint compressedPosition;

void main()
{
	vec3 relativePosition = unpackUnorm4x8(compressedPosition).xyz;
	relativePosition *= brickSize;
	
	vec3 brickOrigin = vec3(0);//TODO

	gl_Position = projection * view * model * vec4(cloudOrigin + brickOrigin + relativePosition, 1);	

}