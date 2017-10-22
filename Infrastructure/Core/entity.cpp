#include "entity.h"
#include "glm/gtc/matrix_transform.hpp"

Entity::~Entity()
{
	for (auto child : m_Children) {
		delete child;
	}

	m_Children.clear();
}

void Entity::SetParent(Entity* parent) noexcept
{
	m_pParent = parent;
}

Entity* Entity::GetParent() const noexcept
{
	return m_pParent;
}

void Entity::AddChild(Entity* child) noexcept
{
	child->SetParent(this);

	m_Children.push_back(child);

	m_XformInvalid = true;
}

void Entity::AddChildren(const std::vector<Entity*>& children) noexcept
{
	m_Children.insert(m_Children.cend(), children.cbegin(), children.cend());

	m_XformInvalid = true;
}

void Entity::SetChildren(const std::vector<Entity*>& children) noexcept
{
	m_Children = children;

	m_XformInvalid = true;
}

const std::vector<Entity*>& Entity::GetChildren() const noexcept
{
	return m_Children;
}

void Entity::SetPosition(const Vec3f& position) noexcept
{
	m_Position = position;

	m_XformInvalid = true;
}

void Entity::SetOrientation(const Quatf& orientation) noexcept
{
	m_Orientation = orientation;

	m_XformInvalid = true;
}

void Entity::SetOrientation(const f32 angle, const Vec3f& axis) noexcept
{
	m_Orientation = glm::rotate(Quatf{}, angle, axis);

	m_XformInvalid = true;
}

const Vec3f& Entity::GetPosition() const noexcept
{
	return m_Position;
}

const Quatf& Entity::GetOrientation() const noexcept
{
	return m_Orientation;
}

const Vec3f& Entity::GetScale() const noexcept
{
	return m_Scale;
}

const Mat4f& Entity::GetXform() const noexcept
{
	return m_Xform;
}

void Entity::Update(const f32 deltaTime) noexcept
{
	// if Xform has to be recalculated...
	if (m_XformInvalid) {

		// Reset identity;
		m_Xform = Mat4f{};

		m_Xform = glm::translate(m_Xform, m_Position);
		m_Xform *= glm::toMat4(m_Orientation);
		m_Xform = glm::scale(m_Xform, m_Scale);

		// Since this Entity's XForm is invalid if it
		// has children they have to be updated.
		for (auto child : m_Children) {
			child->Update(deltaTime);
		}

		m_XformInvalid = false;
	}
}

