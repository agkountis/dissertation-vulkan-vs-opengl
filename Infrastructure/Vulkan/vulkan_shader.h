#ifndef VULKAN_SHADER_H
#define VULKAN_SHADER_H

#include <vulkan/vulkan.h>
#include "resource.h"

class VulkanShader : public Resource {
private:
	VkShaderModule m_ShaderModule{ VK_NULL_HANDLE };

public:
	~VulkanShader() override;

	operator VkShaderModule() noexcept;

	bool Load(const std::string& fileName) noexcept override;
};

#endif //VULKAN_SHADER_H
