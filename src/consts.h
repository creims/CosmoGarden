#ifndef CONSTS_H
#define CONSTS_H

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

// Global meaning
constexpr float PI_F = 3.14159265358979f;

constexpr float CLOSE_TO_0 = 0.001f;
constexpr float CLOSER_TO_0 = 0.0001f;

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
constexpr glm::vec3 WORLD_RIGHT = VEC3_X;

constexpr glm::vec4 VEC4_WORLD_UP = glm::vec4(WORLD_UP, 1.0f);
constexpr glm::vec4 VEC4_WORLD_FRONT = glm::vec4(WORLD_FRONT, 1.0f);
constexpr glm::vec4 VEC4_WORLD_RIGHT = glm::vec4(WORLD_RIGHT, 1.0f);

#endif //CONSTS_H
