#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 v_InLightDirection;
layout(location = 1) in vec3 v_InViewDirection;
layout(location = 2) in vec2 inTexcoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inVertexColor;

layout(push_constant) uniform PushContstants {
    layout(offset = 64) vec4 diffuse;
    layout(offset = 80) vec4 specular;
} pcs;

layout(set = 1, binding = 0) uniform sampler2D diffuseSampler;
layout(set = 1, binding = 1) uniform sampler2D specularSampler;
layout(set = 1, binding = 2) uniform sampler2D normalSampler;

layout(location = 0) out vec4 outColor;

void main()
{
    vec3 n = normalize(texture(normalSampler, inTexcoord).rgb * 2.0 - 1.0);
    vec3 v = normalize(v_InViewDirection);
    vec3 l = normalize(v_InLightDirection);

    vec3 h = normalize(l + v);

    float diffLight = max(dot(n, l), 0.0);

    float specLight = pow(max(dot(n, h), 0.0), 60.0);

    vec4 diffTexel = texture(diffuseSampler, inTexcoord);
    vec4 specTexel = texture(specularSampler, inTexcoord);

	//outColor = diffTexel * pcs.diffuse  * diffLight
	//            + specTexel * pcs.specular * specLight;

	outColor = vec4(inTexcoord, 0.0, 1.0);
}
