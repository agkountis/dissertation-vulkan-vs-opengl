#ifndef DEMO_MATERIAL_H_
#define DEMO_MATERIAL_H_

#include <vulkan_texture.h>

struct DemoMaterial final {
	std::array<VulkanTexture*, SUPPORTED_TEX_COUNT> textures;

	//These are sent to the fragment shader as push constants.
	Vec4f diffuse{ 1.0f, 1.0f, 1.0f, 1.0f};
	Vec4f specular{ 1.0f };

	//Each material has it's own descriptor set.
	VkDescriptorSet descriptorSet;
};

#endif //DEMO_MATERIAL_H_
