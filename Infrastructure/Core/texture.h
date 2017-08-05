#ifndef TEXTURE_H_
#define TEXTURE_H_

#include "resource.h"

enum TextureType {
	TEX_DIFFUSE,
	TEX_SPECULAR,
	TEX_NORMAL,
	SUPPORTED_TEX_COUNT
};

class Texture : public Resource {
private:
	TextureType m_Type;

public:
	explicit Texture(TextureType textureType) : m_Type{ textureType }
	{
	}
};

#endif //TEXTURE_H_
