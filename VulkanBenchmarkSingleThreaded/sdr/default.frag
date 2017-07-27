#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 v_InLightDirection;
layout(location = 1) in vec3 v_InViewDirection;
layout(location = 2) in vec2 inTexcoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inVertexColor;

layout(location = 0) out vec4 outColor;

void main()
{
    vec3 n = normalize(inNormal);
    vec3 v = normalize(v_InViewDirection);
    vec3 l = normalize(v_InLightDirection);

    vec3 h = normalize(l + v);

    float diffLight = max(dot(n, l), 0.0);

    float specLight = pow(max(dot(n, h), 0.0), 60.0);

    vec4 diffColor = vec4(n * 0.5 + 0.5, 1.0);

    vec4 specColor = vec4(1.0);

	outColor = diffColor * diffLight + specColor * specLight;
}
