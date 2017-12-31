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
		assert(glGetError() == GL_NO_ERROR);
		glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
		assert(glGetError() == GL_NO_ERROR);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
		assert(glGetError() == GL_NO_ERROR);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		assert(glGetError() == GL_NO_ERROR);
	}

	const auto& indices = GetIndices();
	if (!indices.empty()) {
		if (!m_Ibo) {
			glGenBuffers(1, &m_Ibo);
			assert(glGetError() == GL_NO_ERROR);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Ibo);
			assert(glGetError() == GL_NO_ERROR);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
			assert(glGetError() == GL_NO_ERROR);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			assert(glGetError() == GL_NO_ERROR);
		}
	}

	glGenVertexArrays(1, &m_Vao);
	assert(glGetError() == GL_NO_ERROR);
	glBindVertexArray(m_Vao);
	assert(glGetError() == GL_NO_ERROR);
	glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
	assert(glGetError() == GL_NO_ERROR);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, tangent)));
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, color)));
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, texcoord)));
	assert(glGetError() == GL_NO_ERROR);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	assert(glGetError() == GL_NO_ERROR);

	glBindVertexArray(0);
	assert(glGetError() == GL_NO_ERROR);

	return true;
}

void GLMesh::Draw() const noexcept
{
	glBindVertexArray(m_Vao);
	assert(glGetError() == GL_NO_ERROR);

	if (m_Ibo) {
		glDrawElements(GL_TRIANGLES, GetIndices().size(), GL_UNSIGNED_INT, GetIndices().data());
		assert(glGetError() == GL_NO_ERROR);
	} else {
		glDrawArrays(GL_TRIANGLES, 0, GetVertices().size());
		assert(glGetError() == GL_NO_ERROR);
	}

	glBindVertexArray(0);
}
