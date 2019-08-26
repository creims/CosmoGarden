#ifndef ARCBALL_H
#define ARCBALL_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include "consts.h"

class Arcball {
public:
    Arcball() = default;

    // Call when sizing/resizing window
    void setBounds(float newWidth, float newHeight);

    // Call when drag starts
    void startDrag(float x, float y);

    // Call when rendering
    void update(float x, float y);

    // Get the current rotation
    glm::quat getRotation();

    // Reset the arcball's rotation
    void reset();

private:
    float adjustWidth{}, adjustHeight{};
    glm::vec3 startVector{};
    glm::quat startRotation{};
    glm::quat currentRotation{QUAT_IDENTITY};

    glm::vec3 mapToSphere(float x, float y);
};


#endif //ARCBALL_H
