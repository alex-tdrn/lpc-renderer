#version 460 core

const float pi = 3.14;
uniform vec3 cloudOrigin;
uniform vec3 brickSize;
uniform uvec3 subdivisions;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int positionSize;
uniform int normalSize;

layout(location = 0) in uint compressedPosition;
layout(location = 1) in uint compressedNormal;
layout(location = 2) in vec3 color;

out VS_OUT
{
	vec3 viewSpacePosition;
	vec3 viewSpaceNormal;
	vec3 modelSpaceNormal;
	vec3 color;
} vs_out;

vec3 decodePosition();
vec3 decodeNormal();

void main()
{
	gl_Position = vec4(decodePosition(), 1.0f);	

	vs_out.viewSpacePosition = vec3(view * model * gl_Position);
	vs_out.modelSpaceNormal = decodeNormal();
	vs_out.viewSpaceNormal = mat3(transpose(inverse(view * model))) * vs_out.modelSpaceNormal;
	vs_out.color = color;
}

vec3 decodePosition()
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

	uint index = gl_BaseInstance;
	uvec3 indices;
	indices.z = index / ((subdivisions.x + 1) * (subdivisions.y + 1));//count surfaces
	index  = index % ((subdivisions.x + 1) * (subdivisions.y + 1));
	indices.y = index / (subdivisions.x + 1);//count lines
	indices.x = index % (subdivisions.x + 1);//count points

	return cloudOrigin + indices * brickSize + relativePosition;
}

vec3 decodeNormal()
{
	vec2 s;
	s.x = float(bitfieldExtract(compressedNormal, 0, normalSize)) / (1 << normalSize);
	s.y = float(bitfieldExtract(compressedNormal, normalSize, normalSize)) / (1 << normalSize);

	float theta = s.y * pi;
    float phi   = (s.x * (2.0 * pi) - pi);

    float sintheta = sin(theta);
    return vec3(sintheta * sin(phi), cos(theta), sintheta * cos(phi));
}