#include "arcball.h"
#include "math_utils.h"

using glm::vec3;
using glm::quat;

void Arcball::setBounds(float newWidth, float newHeight) {
    adjustWidth = 1.0f / ((newWidth - 1.0f) * 0.5f);
    adjustHeight = 1.0f / ((newHeight - 1.0f) * 0.5f);
}

void Arcball::startDrag(float x, float y) {
    // Initialize our reference vector (initial point on sphere)
    startVector = mapToSphere(x, y);

    // Maintain the previous rotation
    startRotation = currentRotation;
}

void Arcball::update(float x, float y) {
    // Map the new point to the sphere
    vec3 currentVector = mapToSphere(x, y);

    // The new rotation is the old rotation times the new rotation (reverse multiplication order)
    currentRotation = rotationBetweenVectors(startVector, currentVector) * startRotation;
}

vec3 Arcball::mapToSphere(float x, float y) {
    float length, norm;
    vec3 result;

    // Transform the mouse coords to [-1..1]
    // and inverse the Y coord
    x = x * adjustWidth - 1.0f;
    y = -(y * adjustHeight - 1.0f);

    length = x * x + y * y;

    // If the point is mapped outside of the sphere
    // (length > radius squared)
    if (length > 1.0f) {
        norm = 1.0f / sqrtf(length);

        // Return the "normalized" vector, a point on the sphere
        result = vec3{x * norm, y * norm, 0.0f};
    } else { // It's inside of the sphere
        // Return a vector to the point mapped inside the sphere
        // where Z = sqrt(radius * radius - length) (our radius is 1.0f though)
        result = normalize(vec3{x, y, sqrtf(1.0f - length)});
    }

    return result;
}

quat Arcball::getRotation() {
    return currentRotation;
}

void Arcball::reset() {
    currentRotation = quat{};
}
