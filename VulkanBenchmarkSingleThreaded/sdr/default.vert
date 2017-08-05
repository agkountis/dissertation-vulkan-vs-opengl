#version 450
#extension GL_ARB_separate_shader_objects : enable

//Vertex attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inColor;
layout(location = 4) in vec2 inTexcoord;

//Uniforms
layout(set = 0, binding = 0) uniform UniformBufferObject {
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
// prefixes: m_ -> model space
//           v_ -> view space
//           t_ -> tangent space
layout(location = 0) out vec3 t_OutlightDirection;
layout(location = 1) out vec3 t_OutViewDirection;
layout(location = 2) out vec2 outTexcoord;
layout(location = 3) out vec3 outNormal;
layout(location = 4) out vec3 outVertexColor;

void main()
{
    //Transform vertex to clipspace.
    vec4 localVertexPosition = vec4(inPosition, 1.0);
    gl_Position = ubo.projection * ubo.view * pushConstant.model * localVertexPosition;

    //Calculate the normal.
    outNormal = normalize(mat3(ubo.view) * inNormal);

	vec3 tangent = normalize(mat3(ubo.view) * inTangent);
	vec3 binormal = normalize(cross(outNormal, tangent));

	mat3 TBN = transpose(mat3(tangent, binormal, outNormal));

    //Move the vertex in view space.
    vec3 v_vertexPosition = (ubo.view * pushConstant.model * localVertexPosition).xyz;

    //Assign the view direction for output.
    t_OutViewDirection = TBN * -v_vertexPosition;

    //Move the light to view space.
//    vec3 v_lightPosition = (ubo.view * vec4(0.0, 0.0, 2.0, 1.0)).xyz;

    vec3 v_lightPosition = (vec4(0.0, 0.0, 2.0, 1.0)).xyz;

    //Calculate and assign the light direction for output.
    t_OutlightDirection = TBN * (v_lightPosition - v_vertexPosition);

    //Assign texture coorinates for output.
    outTexcoord = inTexcoord;

    outVertexColor = inColor;
}
