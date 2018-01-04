#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec2 outTexCoord;

void main()
{
	outTexCoord = vec2((gl_VertexID<< 1) & 2, gl_VertexID & 2);
	gl_Position = vec4(outTexCoord * vec2( 2.0f, -2.0f ) + vec2( -1.0f, 1.0f), 0.0f, 1.0f);
}