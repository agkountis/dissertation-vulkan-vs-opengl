#include "vulkan_entity.h"

VulkanEntity::VulkanEntity(VulkanMesh* mesh)
{
	m_Mesh = mesh;
}

VulkanMaterial& VulkanEntity::GetMaterial() const noexcept
{
	return *m_Material;
}

void VulkanEntity::SetMaterial(VulkanMaterial* material) noexcept
{
	m_Material = material;
}

void VulkanEntity::Draw(VkCommandBuffer commandBuffer) noexcept
{
	if (m_Mesh) {
		m_Mesh->Draw(commandBuffer);
	}
}
