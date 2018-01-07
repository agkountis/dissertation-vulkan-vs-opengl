#include "demo_entity.h"

DemoEntity::DemoEntity(GLMesh* mesh)
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

void DemoEntity::Draw() const noexcept
{
	if (m_Mesh) {
		m_Mesh->Draw();
	}
}

bool DemoEntity::Load(const std::string& fileName) noexcept
{
	return false;
}

