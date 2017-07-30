#include "mesh.h"
#include <cstring>

void Mesh::AddVertex(const Vertex& vertex) noexcept
{
	m_Vertices.push_back(vertex);
}

void Mesh::AddIndex(ui32 index) noexcept
{
	m_Indices.push_back(index);
}

void Mesh::AddVertices(const std::vector<Vertex>& vertices) noexcept
{
	m_Vertices.insert(m_Vertices.cend(), vertices.begin(), vertices.end());
}

void Mesh::AddIndices(const std::vector<ui32>& indices) noexcept
{
	m_Indices.insert(m_Indices.cend(), indices.begin(), indices.end());
}

const std::vector<Vertex>& Mesh::GetVertices() const noexcept
{
	return m_Vertices;
}

const std::vector<ui32>& Mesh::GetIndices() const noexcept
{
	return m_Indices;
}

Vertex* Mesh::GetVertexDataPtr() noexcept
{
	return m_Vertices.data();
}

ui32* Mesh::GetIndexDataPtr() noexcept
{
	return m_Indices.data();
}

bool Mesh::Load(const std::string& fileName) noexcept
{
	return false;
}

