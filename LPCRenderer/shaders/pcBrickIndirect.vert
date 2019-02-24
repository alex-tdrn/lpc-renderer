#version 450 core

uniform vec3 cloudOrigin;
uniform vec3 brickSize;
uniform uvec3 subdivisions;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

layout(location = 0) in uint compressedPosition;

void main()
{
	vec3 relativePosition;
	relativePosition.x = float(bitfieldExtract(compressedPosition, 0, 5)) / 32.0f;
	relativePosition.y = float(bitfieldExtract(compressedPosition, 5, 5)) / 32.0f;
	relativePosition.z = float(bitfieldExtract(compressedPosition, 10, 5)) / 32.0f;
	relativePosition *= brickSize;
	
	uint index = gl_BaseInstanceARB;
	uvec3 indices;
	indices.z = index / ((subdivisions.x + 1) * (subdivisions.y + 1));//count surfaces
	index  = index % ((subdivisions.x + 1) * (subdivisions.y + 1));
	indices.y = index / (subdivisions.x + 1);//count lines
	indices.x = index % (subdivisions.x + 1);//count points

	vec3 brickOrigin = indices * brickSize;

	gl_Position = projection * view * model * vec4(cloudOrigin + brickOrigin + relativePosition, 1);	

}