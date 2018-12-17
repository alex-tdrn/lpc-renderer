#version 450 core

out vec4 fragColor;

in GS_OUT
{
	vec3 color;
} fs_in;

void main()
{
	fragColor = vec4(fs_in.color, 1.0f);
}