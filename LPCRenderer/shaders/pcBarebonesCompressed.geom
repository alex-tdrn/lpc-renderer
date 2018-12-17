#version 450 core
#define MAX_VERTS 256
layout(points, invocations = 32) in;
layout(points, max_vertices = MAX_VERTS) out;

layout(std430, binding = 0) buffer PositionsBuffer
{
	uint positions[];
};

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

in VS_OUT
{
	vec3 size;
	uint bufferOffset;
	uint bufferLength;
} gs_in[];

void main()
{
	for(int i = 0; i < MAX_VERTS; i++)
	{
		uint index = gl_InvocationID * MAX_VERTS + i;
		if(index >= gs_in[0].bufferLength)
			return;
		index += gs_in[0].bufferOffset;
		vec3 position = unpackUnorm4x8(positions[index]).xyz;
		position *= gs_in[0].size;
		gl_Position = projection * view * model * vec4(gl_in[0].gl_Position.xyz + position, 1.0f);
		EmitVertex();
		EndPrimitive();	
	}
}