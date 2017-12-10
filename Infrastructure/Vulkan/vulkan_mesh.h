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

public:
	static VkVertexInputBindingDescription GetVertexInputBindingDescription() noexcept;

	static std::array<VkVertexInputAttributeDescription, 5> GetVertexInputAttributeDescriptions() noexcept;

	bool CreateBuffers() noexcept override;

	void Draw(VkCommandBuffer commandBuffer) noexcept;
};

#endif //VULKAN_MESH_H_
