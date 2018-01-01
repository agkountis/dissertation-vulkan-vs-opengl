#ifndef GL_TEXTURE_H_
#define GL_TEXTURE_H_
#include "resource.h"
#include <GL/glew.h>

class GLTexture final : public Resource {
private:
	GLuint m_Id{ 0 };

public:
	GLTexture() noexcept;

	~GLTexture();

	bool Load(const std::string& fileName) noexcept override;

	GLuint GetId() const noexcept;
};

#endif //GL_TEXTURE_H_