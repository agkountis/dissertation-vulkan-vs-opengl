#version 450
#extension GL_ARB_separate_shader_objects : enable

//Vertex attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inColor;
layout(location = 4) in vec2 inTexcoord;

//Uniforms
layout(binding = 0) uniform UniformBufferObject {
	mat4 model;
	mat4 view;
	mat4 projection;
	mat4 inverseTransposeModelView;
	float time;
} ubo;

out gl_PerVertex {
    vec4 gl_Position;
};

// Varying variables
// prefixes: m_ -> model space
//           v_ -> view space
//           t_ -> tangent space
layout(location = 0) out vec3 v_OutlightDirection;
layout(location = 1) out vec3 v_OutViewDirection;
layout(location = 2) out vec2 outTexcoord;
layout(location = 3) out vec3 outNormal;
layout(location = 4) out vec3 outVertexColor;

void main()
{
    //Transform vertex to clipspace.
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(inPosition, 1.0);

    //Calculate the normal.
    outNormal = normalize(ubo.inverseTransposeModelView * vec4(inNormal, 0.0)).xyz;

    //Move the vertex in view space.
    vec3 v_vertexPosition = (ubo.view * vec4(inPosition, 1.0)).xyz;

    //Assign the view direction for output.
    v_OutViewDirection = -v_vertexPosition;

    //Move the light to view space.
    vec3 v_lightPosition = (ubo.view * vec4(0.0, 0.0, 5.0, 1.0)).xyz;

    //Calculate and assign the light direction for output.
    v_OutlightDirection = v_lightPosition - v_vertexPosition;

    //Assign texture coorinates for output.
    outTexcoord = inTexcoord;

    outVertexColor = inColor;
}
