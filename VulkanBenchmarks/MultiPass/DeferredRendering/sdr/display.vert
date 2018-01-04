#version 450 core
//#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec2 outTexCoord;

void main()
{
	//Vulkan Version
	// Draws a triangle that fills the whole screen. 3 VS invocations only and gets turned into a quad due to clipping.
	// tc[0, 0] is at the top left.
	outTexCoord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(outTexCoord * 2.0f - 1.0f, 0.0f, 1.0f);

	//GL version
	//outTexCoord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	//gl_Position = vec4(outTexCoord * vec2( 2.0f, -2.0f ) + vec2( -1.0f, 1.0f), 0.0f, 1.0f);
}