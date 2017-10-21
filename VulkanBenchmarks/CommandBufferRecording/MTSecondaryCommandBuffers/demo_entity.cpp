#include "demo_entity.h"

DemoEntity::DemoEntity(VulkanMesh* mesh)
{
	m_Mesh = mesh;
}

DemoMaterial& DemoEntity::GetMaterial() noexcept
{
	return m_Material;
}

void DemoEntity::SetMaterial(const DemoMaterial& material) noexcept
{
	m_Material = material;
}

void DemoEntity::Draw(VkCommandBuffer commandBuffer) noexcept
{
	if (m_Mesh) {
		m_Mesh->Draw(commandBuffer);
	}
}

bool DemoEntity::Load(const std::string& fileName) noexcept
{
	return false;
}

