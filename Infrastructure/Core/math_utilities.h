#ifndef MATH_UTILITIES_H_
#define MATH_UTILITIES_H_

#include "types.h"
#include "glm/gtc/matrix_transform.hpp"

Mat4f Translate(const Mat4f& mat, const Vec3f& position) noexcept
{
	return glm::translate(mat, position);
}

Mat4f Rotate(const Mat4f& mat, float angle, const Vec3f& axis) noexcept
{
	return glm::rotate(mat, angle, axis);
}

Mat4f Scale(const Mat4f& mat, const Vec3f& scale) noexcept
{
	return glm::scale(mat, scale);
}

Mat4f LookAt(const Vec3f& target, const Vec3f& origin, const Vec3f& up) noexcept
{
	return glm::lookAt(target, origin, up);
}

Mat4f Perspective(f32 fov, f32 aspect, f32 nearPlane, f32 farPlane) noexcept
{
	return glm::perspective(fov, aspect, nearPlane, farPlane);
}

Mat4f Inverse(const Mat4f& mat) noexcept
{
	return glm::inverse(mat);
}

Mat4f Transpose(const Mat4f& mat) noexcept
{
	return glm::transpose(mat);
}

f32 ToRadians(f32 degrees) noexcept
{
	return glm::radians(degrees);
}

f32 ToDegrees(f32 radians) noexcept
{
	return glm::degrees(radians);
}

#endif //MATH_UTILITIES_H_
