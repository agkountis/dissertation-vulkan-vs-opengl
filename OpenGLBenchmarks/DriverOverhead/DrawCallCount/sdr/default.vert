#version 450 core
#extension GL_ARB_separate_shader_objects : enable

//Vertex attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inColor;
layout(location = 4) in vec2 inTexcoord;

layout(location = 5) uniform mat4 MVP;

out gl_PerVertex {
    vec4 gl_Position;
};

layout(location = 0) out vec2 outTexCoord;

void main()
{
	//const vec4 vertices[3] = vec4[3](vec4(0.25, -0.25, 0.5 ,1.0),
									 //vec4(-0.25, -0.25, 0.5, 1.0),
									 //vec4(0.25, 0.25, 0.5, 1.0));

	//gl_Position = vertices[gl_VertexID];

	//outTexCoord = vec2((gl_VertexID<< 1) & 2, gl_VertexID & 2);
	//gl_Position = vec4(outTexCoord * vec2( 2.0f, -2.0f ) + vec2( -1.0f, 1.0f), 0.0f, 1.0f);

	gl_Position = MVP * vec4(inPosition, 1.0);
	//outTexCoord = inTexcoord;
}
