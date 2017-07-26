#ifndef VULKAN_SHADER_H
#define VULKAN_SHADER_H

#include <vulkan/vulkan.h>
#include "resource.h"

class VulkanShader : public Resource {
private:
	VkDevice m_pLogicalDevice{ nullptr };

	VkShaderModule m_ShaderModule{ nullptr };

public:
	VulkanShader(VkDevice logicalDevice);

	~VulkanShader();

	operator VkShaderModule() noexcept;

	bool Load(const std::string& fileName) noexcept override;
};

#endif //VULKAN_SHADER_H
