#ifndef VERTEX_H_
#define VERTEX_H_

#include "types.h"

struct Vertex {
	Vec3f position;
	Vec3f normal;
	Vec3f tangent;
	Vec3f color;
	Vec2f texcoord;
};

#endif //VERTEX_H_
