#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include "Camera.h"

constexpr float MAX_VERTICAL_ANGLE = 85.0f;

glm::mat4 Camera::matrix() const {
    return _projection * orientation() * glm::translate(glm::mat4{1.0f}, -_position);
}

Camera::Camera(float aspectRatio) {
    setAspectRatio(aspectRatio);
}

// TODO: expose fov, clip planes
void Camera::setAspectRatio(float aspectRatio) {
    _projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 1000.0f);
    lookAt(VEC3_ORIGIN);
}

void Camera::lookAt(glm::vec3 target) {
    glm::vec3 direction = glm::normalize(target - _position);
    _verticalAngle = glm::radians(asinf(-direction.y));
    _horizontalAngle = -glm::radians(atan2f(-direction.x, -direction.z));
    normalizeAngles();
}

glm::vec3 Camera::position() const {
    return _position;
}

glm::mat4 Camera::orientation() const {
    glm::mat4 orientation{1.0f};
    orientation = glm::rotate(orientation, glm::radians(_verticalAngle), WORLD_RIGHT);
    orientation = glm::rotate(orientation, glm::radians(_horizontalAngle), WORLD_UP);
    return orientation;
}

void Camera::normalizeAngles() {
    _horizontalAngle = fmodf(_horizontalAngle, 360.0f);
    if(_horizontalAngle < 0.0f) _horizontalAngle += 360.0f;
    if(_verticalAngle > MAX_VERTICAL_ANGLE) _verticalAngle = MAX_VERTICAL_ANGLE;
    else if(_verticalAngle < -MAX_VERTICAL_ANGLE) _verticalAngle = -MAX_VERTICAL_ANGLE;
}

glm::vec3 Camera::front() const {
    glm::vec4 front = glm::inverse(orientation()) * -VEC4_WORLD_FRONT;
    return glm::vec3(front);
}

glm::vec3 Camera::up() const {
    return glm::vec3(glm::inverse(orientation()) * -VEC4_WORLD_UP);
}

glm::vec3 Camera::right() const {
    return glm::vec3(glm::inverse(orientation()) * -VEC4_WORLD_RIGHT);
}

void Camera::offsetPosition(const glm::vec3& offset) {
    _position += offset;
}

void Camera::offsetOrientation(float horizontal, float vertical) {
    _horizontalAngle += horizontal;
    _verticalAngle += vertical;
    normalizeAngles();
}

