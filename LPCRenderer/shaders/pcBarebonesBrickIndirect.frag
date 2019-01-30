#version 460 core

uniform vec3 diffuseColor;
out vec4 fragColor;

void main()
{
	if(length(gl_PointCoord - vec2(0.5f)) > 0.5f)
		discard;
	fragColor = vec4(diffuseColor, 1.0);
}