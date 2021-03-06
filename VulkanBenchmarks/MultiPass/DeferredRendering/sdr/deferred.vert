#version 450 core
#extension GL_ARB_separate_shader_objects : enable
//#extension GL_ARB_shading_language_420pack : enable

//Vertex attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inColor;
layout(location = 4) in vec2 inTexcoord;

//Uniforms
layout(std140, set = 0, binding = 0) uniform UniformBufferObject {
	mat4 view;
	mat4 projection;
} ubo;

layout(push_constant) uniform PushContstants {
    mat4 model;
} pushConstant;

out gl_PerVertex {
    vec4 gl_Position;
};

// Varying variables
// prefixes: w_ -> world space
//           v_ -> view space
//           t_ -> tangent space
layout(location = 0) out vec2 outTexcoord;
layout(location = 1) out vec4 w_outPosition;
layout(location = 2) out vec3 w_outNormal;
layout(location = 3) out vec3 w_outTangent;

void main()
{
    //Transform vertex to clipspace.
    vec4 localVertexPosition = vec4(inPosition, 1.0);
    gl_Position = ubo.projection * ubo.view * pushConstant.model * localVertexPosition;

	w_outPosition = pushConstant.model * localVertexPosition;

    mat3 normalMatrix = transpose(inverse(mat3(pushConstant.model)));


	w_outNormal = normalMatrix * normalize(inNormal);
	w_outTangent = normalMatrix * normalize(inTangent);

    //Assign texture coorinates for output.
    outTexcoord = inTexcoord;
}
