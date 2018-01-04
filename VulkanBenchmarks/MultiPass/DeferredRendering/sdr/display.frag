#version 450 core
#extension GL_ARB_separate_shader_objects : enable
//#extension GL_ARB_shading_language_420pack : enable

layout(binding = 0) uniform sampler2D positionSampler;
layout(binding = 1) uniform sampler2D normalSampler;
layout(binding = 2) uniform sampler2D albedoSampler;
layout(binding = 3) uniform sampler2D specularSampler;
layout(binding = 4) uniform sampler2D depthSampler;

const int lightCount = 4;

layout(std140, set = 0, binding = 5) uniform Lights {
	vec4[lightCount] w_Positions;
	vec4[lightCount] colors;
	vec4[lightCount] radi;
	vec3 w_eyePos;
} lightsUbo;

layout(push_constant) uniform PushConstant {
    int attachmentIndex;
} pc;

layout(location = 0) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;


vec4 linearizeDepth(vec2 uv)
{
    float zNear = 1.0;    // TODO: Replace by the zNear of your perspective projection
    float zFar  = 2000.0; // TODO: Replace by the zFar  of your perspective projection
    float depth = texture(depthSampler, uv).x;
    return vec4(vec3((2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear))), 1.0);
}


vec4 shade(vec4 w_Pos, vec3 normal, vec4 albedo, vec4 specular)
{
    vec3 color = vec3(0.0, 0.0, 0.0);

    color += albedo.rgb * vec3(0.1, 0.1, 0.1);

    for(int i = 0; i < lightCount; ++i) {
		// Vector to light
		vec3 L = lightsUbo.w_Positions[i].xyz - w_Pos.xyz;

		// Distance from light to fragment position
		float dist = length(L);

		// Viewer to fragment
		vec3 V = normalize(lightsUbo.w_eyePos - w_Pos.xyz);

        float lightRadius = lightsUbo.radi[i].x;
		if(dist < lightRadius) {
			// Light to fragment
			L = normalize(L);

			// Attenuation
			float atten = lightRadius / (pow(dist, 2.0) + 1.0);

            vec3 H = normalize(L + V);

			// Diffuse
			vec3 N = normalize(normal);
			float NdotL = max(0.0, dot(N, L));
			color += lightsUbo.colors[i].rgb * albedo.rgb * NdotL * atten;

			// Specular
			float NdotH = max(0.0, dot(N, H));
			color += lightsUbo.colors[i].rgb * specular.rgb * pow(NdotH, 16.0) * atten;
		}
	}

	return vec4(color, 1.0);
}

void main()
{
	vec4 w_Pos = texture(positionSampler, inTexCoord);
	vec4 normal = texture(normalSampler, inTexCoord);

	vec4 albedo = texture(albedoSampler, inTexCoord);
    vec4 specular = texture(specularSampler, inTexCoord);

    switch (pc.attachmentIndex) {
        case 0:
    	    outColor = shade(w_Pos, normal.xyz, albedo, specular);
            break;
        case 1:
            outColor = w_Pos;
            break;
        case 2:
            outColor = normal;
            break;
        case 3:
            outColor = albedo;
            break;
        case 4:
            outColor = specular;
			break;
		case 5:
			outColor = linearizeDepth(inTexCoord);
            break;
    }

}
