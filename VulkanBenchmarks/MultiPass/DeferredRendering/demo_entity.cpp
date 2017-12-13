#include "demo_entity.h"

DemoEntity::DemoEntity(VulkanMesh* mesh)
{
	m_Mesh = mesh;
}

DemoMaterial* DemoEntity::GetMaterial() noexcept
{
	return m_Material;
}

void DemoEntity::SetName(const std::string& name) noexcept
{
	m_Name = name;
}

void DemoEntity::SetMesh(VulkanMesh* mesh) noexcept
{
	m_Mesh = mesh;
}

VulkanMesh* DemoEntity::GetMesh() const noexcept
{
	return m_Mesh;
}

void DemoEntity::SetMaterial(DemoMaterial* material) noexcept
{
	m_Material = material;
}

bool DemoEntity::Load(const std::string& fileName) noexcept
{
	return false;
}

