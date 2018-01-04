#ifndef GL_BUFFER_H_
#define GL_BUFFER_H_
#include "GL/glew.h"
#include <cstring>

template<typename T>
class GLBuffer final {
private:
	GLuint m_Id{ 0 };

	void* m_Data{ nullptr };

public:
	~GLBuffer()
	{
		glDeleteBuffers(1, &m_Id);
	}

	void Create() noexcept
	{
		glCreateBuffers(1, &m_Id);
		glNamedBufferData(m_Id, sizeof(T), nullptr, GL_DYNAMIC_DRAW);
	}

	void Map(const GLenum accessMode) noexcept
	{
		m_Data = glMapNamedBuffer(m_Id, accessMode);
	}

	void Unmap() const noexcept
	{
		glUnmapNamedBuffer(m_Id);
	}

	void Fill(const T* data) noexcept
	{
		memcpy(m_Data, data, sizeof(T));
	}

	GLuint GetId() const noexcept
	{
		return m_Id;
	}
};

#endif //GL_BUFFER_H_
