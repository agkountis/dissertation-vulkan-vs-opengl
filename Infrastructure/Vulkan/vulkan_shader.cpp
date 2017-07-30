#include <vector>
#include <fstream>
#include "vulkan_shader.h"
#include "logger.h"
#include "vulkan_infrastructure_context.h"

VulkanShader::~VulkanShader()
{
	vkDestroyShaderModule(G_VulkanDevice, m_ShaderModule, nullptr);
}

VulkanShader::operator VkShaderModule() noexcept
{
	return m_ShaderModule;
}

bool VulkanShader::Load(const std::string& fileName) noexcept
{
	std::ifstream file{ fileName, std::ios::ate | std::ios::binary };

	if (!file.is_open()) {
		ERROR_LOG("Failed to open file: " + fileName);
		return false;
	}

	i64 fileSize{ file.tellg() };

	assert(fileSize > 0);

	std::vector<Byte> buffer;
	buffer.resize(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	VkShaderModuleCreateInfo shaderModuleCreateInfo{};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = static_cast<ui32>(buffer.size());
	shaderModuleCreateInfo.pCode = reinterpret_cast<const ui32*>(buffer.data());

	VkResult result{ vkCreateShaderModule(G_VulkanDevice, &shaderModuleCreateInfo, nullptr, &m_ShaderModule) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to load vulkan shader: " + fileName +". Reason: Failed to create shader module.");
		return false;
	}

	return true;
}
