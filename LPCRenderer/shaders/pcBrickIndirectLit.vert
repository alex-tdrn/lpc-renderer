#version 450 core

uniform vec3 cloudOrigin;
uniform vec3 brickSize;
uniform uvec3 subdivisions;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int positionSize;

layout(location = 0) in uint compressedPosition;
layout(location = 1) in vec3 normal;

out VS_OUT
{
	vec3 viewSpacePosition;
	vec3 viewSpaceNormal;
	vec3 modelSpaceNormal;
} vs_out;

void main()
{
	vec3 relativePosition;
	if(positionSize == 16)
	{
		relativePosition.x = float(bitfieldExtract(compressedPosition, 0, 5)) / 32.0f;
		relativePosition.y = float(bitfieldExtract(compressedPosition, 5, 5)) / 32.0f;
		relativePosition.z = float(bitfieldExtract(compressedPosition, 10, 5)) / 32.0f;
	}
	else if (positionSize == 32)
	{
		relativePosition.x = float(bitfieldExtract(compressedPosition, 0, 10)) / 1024.0f;
		relativePosition.y = float(bitfieldExtract(compressedPosition, 10, 10)) / 1024.0f;
		relativePosition.z = float(bitfieldExtract(compressedPosition, 20, 10)) / 1024.0f;
	}
	relativePosition *= brickSize;
	
	uint index = gl_BaseInstanceARB;
	uvec3 indices;
	indices.z = index / ((subdivisions.x + 1) * (subdivisions.y + 1));//count surfaces
	index  = index % ((subdivisions.x + 1) * (subdivisions.y + 1));
	indices.y = index / (subdivisions.x + 1);//count lines
	indices.x = index % (subdivisions.x + 1);//count points

	vec3 brickOrigin = indices * brickSize;

	gl_Position = vec4(cloudOrigin + brickOrigin + relativePosition, 1.0f);	

	vs_out.viewSpacePosition = vec3(view * model * gl_Position);
	vs_out.viewSpaceNormal = mat3(transpose(inverse(view * model))) * normal;

	vs_out.modelSpaceNormal = normal;


}