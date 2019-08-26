#ifndef CAMERA_H
#define CAMERA_H

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "consts.h"


class Camera {
public:
    Camera() = default;
    explicit Camera(float aspectRatio);

    void setAspectRatio(float aspectRatio);
    void lookAt(glm::vec3 target);
    void offsetPosition(const glm::vec3& offset);
    void offsetOrientation(float horizontal, float vertical);

    glm::mat4 orientation() const;
    glm::mat4 matrix() const;
    glm::vec3 position() const;

    glm::vec3 front() const;
    glm::vec3 up() const;
    glm::vec3 right() const;

private:
    glm::mat4 _projection{1.0f};
    float _horizontalAngle{0.0f}, _verticalAngle{0.0f};
    glm::vec3 _position{glm::vec3{0.0f, 0.0f, 15.0f}};

    void normalizeAngles();
};


#endif //CAMERA_H
