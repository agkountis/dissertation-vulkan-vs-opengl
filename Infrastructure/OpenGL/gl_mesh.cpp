#include "gl_mesh.h"
#include "logger.h"

GLMesh::~GLMesh()
{
	glDeleteBuffers(1, &m_Vbo);
	glDeleteBuffers(1, &m_Ibo);
}

bool GLMesh::CreateBuffers() noexcept
{
	const auto& vertices = GetVertices();
	if (vertices.empty()) {
		ERROR_LOG("Cannot create VBO. The mesh has no vertices.");
		return false;
	}

	if (!m_Vbo) {
		glGenBuffers(1, &m_Vbo);
		glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	const auto& indices = GetIndices();
	if (!indices.empty()) {
		if (!m_Ibo) {
			glGenBuffers(1, &m_Ibo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Ibo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}
	}

	glGenVertexArrays(1, &m_Vao);
	glBindVertexArray(m_Vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, tangent)));
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, color)));
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, texcoord)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);

	glBindVertexArray(0);

	return true;
}

void GLMesh::Draw() const noexcept
{
	glBindVertexArray(m_Vao);

	if (m_Ibo) {
		glDrawElements(GL_TRIANGLES, GetIndices().size(), GL_UNSIGNED_INT, GetIndices().data());
	} else {
		glDrawArrays(GL_TRIANGLES, 0, GetVertices().size());
	}

	glBindVertexArray(0);
}
