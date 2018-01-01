#include "gl_texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "logger.h"

GLTexture::GLTexture() noexcept
{
	glCreateTextures(GL_TEXTURE_2D, 1, &m_Id);
}

GLTexture::~GLTexture()
{
	glDeleteTextures(1, &m_Id);
}

bool GLTexture::Load(const std::string& fileName) noexcept
{
	Vec2i size;
	int colorChannels;

	stbi_uc* pixels{
		stbi_load(fileName.c_str(),
		          &size.x,
		          &size.y,
		          &colorChannels,
		          STBI_rgb_alpha)
	};

	if (!pixels) {
		ERROR_LOG("Failed to load image: " + fileName);
		return false;
	}

	// specify the texture storage type
	glTextureStorage2D(m_Id, 1, GL_RGBA8, size.x, size.y);

	auto err = glGetError();

	// fill it with data.
	glTextureSubImage2D(m_Id, 0, 0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	stbi_image_free(pixels);

	return true;
}

GLuint GLTexture::GetId() const noexcept
{
	return m_Id;
}
