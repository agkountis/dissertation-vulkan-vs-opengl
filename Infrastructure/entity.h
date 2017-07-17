#ifndef ENTITY_H_
#define ENTITY_H_
#include "types.h"
#include <string>

class Entity {
private:
	Mat4f m_Xform;

public:
	void SetPosition(const Vec3f& position) noexcept;

	void SetRotation(const Quatf& rotation) noexcept;

	void SetRotation(f32 angle, const Vec3f& axis) noexcept;

	bool Load(const std::string& fileName) noexcept;
};

#endif //MESH_H_
