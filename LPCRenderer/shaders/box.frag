#version 420 core

in float occupancy;
out vec4 fragColor;

void main()
{
	if(occupancy > 1.0f)
		fragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	else
		fragColor = vec4(mix(vec3(1.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), occupancy), 1.0f);
}