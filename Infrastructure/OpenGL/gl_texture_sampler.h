#ifndef GL_TEXTURE_SAMPLER_H_
#define GL_TEXTURE_SAMPLER_H_
#include <GL/glew.h>
#include "types.h"

struct GLTextureSamplerCreateInfo {
	GLint minFilter;
	GLint magFilter;
	GLint wrapS;
	GLint wrapT;
	GLint wrapR;
	Vec4f borderColor;
};

class GLTextureSampler final {
private:
	GLuint m_Id{ 0 };

public:
	~GLTextureSampler();

	bool Create(const GLTextureSamplerCreateInfo& info) noexcept;

	GLuint GetId() const noexcept;
};

#endif //GL_TEXTURE_SAMPLER_H_
