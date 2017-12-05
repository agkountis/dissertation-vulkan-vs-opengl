#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform sampler2D positionSampler;
layout(binding = 1) uniform sampler2D normalSampler;
layout(binding = 2) uniform sampler2D albedoSampler;
layout(binding = 3) uniform sampler2D specularSampler;

layout(set = 0, binding = 4) uniform Lights {
	vec3[4] positions;
	vec4[4] colors;
	//TODO
} lightsUbo;

layout(location = 0) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

void main()
{
	vec4 v_Pos = texture(positionSampler, inTexCoord);
	vec4 normal = texture(normalSampler, inTexCoord);
	vec4 albedo = texture(albedoSampler, inTexCoord);
	vec4 specular = texture(specularSampler, inTexCoord);

	outColor = albedo;
}
