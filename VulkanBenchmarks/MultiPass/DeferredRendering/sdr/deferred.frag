#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 inTexcoord;
layout(location = 1) in vec4 v_inPosition;

layout(push_constant) uniform PushContstants {
    layout(offset = 64) vec4 diffuse;
    layout(offset = 80) vec4 specular;
} pcs;

layout(set = 1, binding = 0) uniform sampler2D diffuseSampler;
layout(set = 1, binding = 1) uniform sampler2D specularSampler;
layout(set = 1, binding = 2) uniform sampler2D normalSampler;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo;
layout(location = 3) out vec4 outSpecular;

void main()
{
	outPosition = v_inPosition;
	outNormal = vec4(normalize(texture(normalSampler, inTexcoord).rgb * 2.0 - 1.0), 1.0);
	outAlbedo = texture(diffuseSampler, inTexcoord) * pcs.diffuse;
	outSpecular = texture(specularSampler, inTexcoord) * pcs.specular;
}
