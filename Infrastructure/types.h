#ifndef TYPES_H_
#define TYPES_H_
#include "glm/glm.hpp"
#include <cstdint>

using Vec2f = glm::vec2;
using Vec3f = glm::vec3;
using Vec4f = glm::vec4;

using Vec2i = glm::ivec2;
using Vec3i = glm::ivec3;
using Vec4i = glm::ivec4;

using Vec2ui = glm::u32vec2;
using Vec3ui = glm::u32vec3;
using Vec4ui = glm::u32vec4;

using Quatf = glm::quat;
using Quatd = glm::dquat;

using Mat4f = glm::mat4;
using Mat3f = glm::mat3;

using Mat4d = glm::dmat4;
using Mat3d = glm::dmat3;

using f32 = float;
using f64 = double;

using ui64 = uint64_t;
using ui32 = uint32_t;
using ui16 = uint16_t;
using ui8 = uint8_t;

using Byte = char;

using i64 = int64_t;
using i32 = int32_t;
using i16 = int16_t;
using i8 = int8_t;

#endif // TYPES_H_
