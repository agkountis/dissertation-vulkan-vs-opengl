#version 450
#extension GL_ARB_separate_shader_objects : enable

//Vertex attributes
//layout(location = 0) in vec3 inPosition;
//layout(location = 1) in vec3 inNormal;
//layout(location = 2) in vec3 inTangent;
//layout(location = 3) in vec3 inColor;
//layout(location = 4) in vec2 inTexcoord;

layout(location = 0) out vec2 outTexCoord;

void main()
{
	outTexCoord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(outTexCoord * vec2( 2.0f, -2.0f ) + vec2( -1.0f, 1.0f), 0.0f, 1.0f);
}