#ifndef ENTITY_H_
#define ENTITY_H_
#include "resource.h"
#include "timer.h"
#include <string>
#include <vector>

class Entity : public Resource {
private:
	Vec3f m_Position;

	Quatf m_Orientation;

	Vec3f m_Scale{1.0f};

	Mat4f m_Xform{1.0f};

	bool m_XformInvalid{ true };

	Entity* m_pParent{ nullptr };

	std::vector<Entity*> m_Children;

public:
	~Entity() override;

	void SetParent(Entity* parent) noexcept;

	Entity* GetParent() const noexcept;

	void AddChild(Entity* child) noexcept;

	void AddChildren(const std::vector<Entity*>& children) noexcept;

	void SetChildren(const std::vector<Entity*>& children) noexcept;

	const std::vector<Entity*>& GetChildren() const noexcept;

	void SetPosition(const Vec3f& position) noexcept;

	void SetOrientation(const Quatf& orientation) noexcept;

	void SetOrientation(f32 angle, const Vec3f& axis) noexcept;

	const Vec3f& GetPosition() const noexcept;

	const Quatf& GetOrientation() const noexcept;

	const Vec3f& GetScale() const noexcept;

	const Mat4f& GetXform() const noexcept;

	virtual void Update(f32 deltaTime) noexcept;
};

#endif //ENTITY_H_
