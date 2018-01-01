#include "gl_texture_sampler.h"
#include "types.h"
#include "logger.h"

GLTextureSampler::~GLTextureSampler()
{
	glDeleteSamplers(1, &m_Id);
}

bool GLTextureSampler::Create(const GLTextureSamplerCreateInfo& info) noexcept
{
//GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T, GL_TEXTURE_WRAP_R, GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER, GL_TEXTURE_BORDER_COLOR, GL_TEXTURE_MIN_LOD, GL_TEXTURE_MAX_LOD, GL_TEXTURE_LOD_BIAS GL_TEXTURE_COMPARE_MODE, or GL_TEXTURE_COMPARE_FUNC.
	glCreateSamplers(1, &m_Id);

	glSamplerParameteri(m_Id, GL_TEXTURE_MIN_FILTER, info.minFilter);
	glSamplerParameteri(m_Id, GL_TEXTURE_MAG_FILTER, info.magFilter);
	glSamplerParameteri(m_Id, GL_TEXTURE_WRAP_S, info.wrapS);
	glSamplerParameteri(m_Id, GL_TEXTURE_WRAP_T, info.wrapT);
	glSamplerParameteri(m_Id, GL_TEXTURE_WRAP_R, info.wrapR);
	glSamplerParameterfv(m_Id, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(info.borderColor));

	const auto err = glGetError();

	if (err != GL_NO_ERROR) {
		ERROR_LOG("Failed to create texture sampler. Error code: " + std::to_string(err));
		return false;
	}

	return true;
}

GLuint GLTextureSampler::GetId() const noexcept
{
	return m_Id;
}
