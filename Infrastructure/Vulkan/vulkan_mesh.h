#ifndef VULKAN_MESH_H_
#define VULKAN_MESH_H_

#include <vulkan/vulkan.h>
#include <array>
#include "mesh.h"
#include "vulkan_device.h"

class VulkanMesh final : public Mesh {
private:
	VulkanBuffer m_Vbo;

	VulkanBuffer m_Ibo;

	ui32 m_MaterialIndex{ 0 };

public:
	static VkVertexInputBindingDescription GetVertexInputBindingDescription() noexcept;

	static std::array<VkVertexInputAttributeDescription, 5> GetVertexInputAttributeDescriptions() noexcept;

	bool CreateBuffers() noexcept override;

	void Draw(VkCommandBuffer commandBuffer) noexcept;

	void SetMaterialIndex(const ui32 materialIndex) noexcept
	{
		m_MaterialIndex = materialIndex;
	}

	ui32 GetMaterialIndex() const noexcept
	{
		return m_MaterialIndex;
	}
};

#endif //VULKAN_MESH_H_
