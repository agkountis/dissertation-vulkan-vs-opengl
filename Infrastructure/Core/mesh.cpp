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

void Mesh::FlipNormals() noexcept
{
	for (auto& vertex : m_Vertices) {
		vertex.normal = -vertex.normal;
	}
}

void Mesh::GenerateIndices(VertexWinding vertexWinding) noexcept
{
	if (!m_Indices.empty()) {
		return;
	}

	ui64 quad_count = m_Vertices.size() / 4;
	ui64 triangle_count = quad_count * 2;

	m_Indices.resize(triangle_count * 3);

	for (ui32 i = 0, j = 0; i < m_Indices.size(); i += 6, j += 4) {
		m_Indices[i] = j;
		switch (vertexWinding) {
			case VertexWinding::CLOCKWISE:
				m_Indices[i + 1] = m_Indices[i + 4] = j + 1;
				m_Indices[i + 2] = m_Indices[i + 3] = j + 2;
				break;
			case VertexWinding::COUNTERCLOCKWISE:
				m_Indices[i + 1] = m_Indices[i + 4] = j + 2;
				m_Indices[i + 2] = m_Indices[i + 3] = j + 1;
				break;
		}

		m_Indices[i + 5] = j + 3;
	}
}

