#version 450 core

uniform vec3 diffuseColor;
out vec4 fragColor;

in VS_OUT
{
	vec3 color;
} vs_in;

void main()
{
	if(length(gl_PointCoord - vec2(0.5f)) > 0.5f)
		discard;
	fragColor = vec4(vs_in.color, 1.0);
}