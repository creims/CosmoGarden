#ifndef CONSTS_H
#define CONSTS_H

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

// Global meaning
constexpr float PI_F = 3.14159265358979f;

constexpr glm::vec3 VEC3_ORIGIN{0.0f, 0.0f, 0.0f};
constexpr glm::vec3 VEC3_X{1.0f, 0.0f, 0.0f};
constexpr glm::vec3 VEC3_Y{0.0f, 1.0f, 0.0f};
constexpr glm::vec3 VEC3_Z{0.0f, 0.0f, 1.0f};

constexpr glm::quat QUAT_IDENTITY{1.0f, 0.0f, 0.0f, 0.0f};
constexpr glm::quat QUAT_90X{0.7071f, 0.0f, 0.0f, 0.7071f};
constexpr glm::quat QUAT_90Y{0.0f, 0.7071f, 0.0f, 0.7071f};
constexpr glm::quat QUAT_90Z{0.0f, 0.0f, 0.7071f, 0.7071f};

// Contextual
constexpr int MS_PER_TICK = 50;
constexpr float TICKS_PER_MS = 1.0f / MS_PER_TICK;

constexpr glm::vec3 WORLD_UP = VEC3_Y;
constexpr glm::vec3 WORLD_FRONT = VEC3_Z;

#endif //CONSTS_H
