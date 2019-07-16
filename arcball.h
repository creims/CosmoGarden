#ifndef ARCBALL_H
#define ARCBALL_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

class Arcball {
public:
    Arcball() = default;

    // Call when sizing/resizing window
    void setBounds(float newWidth, float newHeight);

    // Call when drag starts
    void startDrag(float x, float y, glm::quat currentRotation);

    // Call when rendering
    glm::quat update(float x, float y);

private:
    float adjustWidth{}, adjustHeight{};
    glm::vec3 startVector{};
    glm::quat startRotation{};

    glm::vec3 mapToSphere(float x, float y);
};


#endif //ARCBALL_H
