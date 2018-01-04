#include "demo_entity.h"

DemoEntity::DemoEntity(GLMesh* mesh)
{
	m_Mesh = mesh;
}

void DemoEntity::SetName(const std::string& name) noexcept
{
	m_Name = name;
}

DemoMaterial* DemoEntity::GetMaterial() const noexcept
{
	return m_Material;
}

void DemoEntity::SetMaterial(DemoMaterial* material) noexcept
{
	m_Material = material;
}

void DemoEntity::SetMesh(GLMesh* mesh) noexcept
{
	m_Mesh = mesh;
}

GLMesh* DemoEntity::GetMesh() const noexcept
{
	return m_Mesh;
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

