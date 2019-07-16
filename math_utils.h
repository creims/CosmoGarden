#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>

glm::vec3 orthogonal(glm::vec3 v);

glm::quat rotationBetweenVectors(glm::vec3 start, glm::vec3 dest);
glm::quat lookAt(glm::vec3 dir, glm::vec3 up);

#endif //MATH_UTILS_H
