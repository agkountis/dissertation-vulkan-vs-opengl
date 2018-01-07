#ifndef DEMO_MATERIAL_H_
#define DEMO_MATERIAL_H_
#include "texture.h"
#include <array>

class GLTexture;

struct DemoMaterial final {
	std::array<GLTexture*, SUPPORTED_TEX_COUNT> textures;

	//These are sent to the fragment shader as push constants.
	Vec4f diffuse{ 1.0f, 1.0f, 1.0f, 1.0f};
	Vec4f specular{ 1.0f };
};

#endif //DEMO_MATERIAL_H_
