// Adapted from https://github.com/opengl-tutorials/ogl/blob/master/common/quaternion_utils.cpp
#include "math_utils.h"
#include "consts.h"

#include <iostream>

#define CLOSE_TO_0 0.001f
#define CLOSER_TO_0 0.0001f

using glm::vec3;
using glm::quat;

/* Always works if the input is non-zero.
 * Doesn’t require the input to be normalised.
 * Doesn’t normalise the output.
 * From http://lolengine.net/blog/2013/09/21/picking-orthogonal-vector-combing-coconuts
 * */
vec3 orthogonal(vec3 const v)
{
    return abs(v.x) > abs(v.z) ? vec3(-v.y, v.x, 0.0)
                               : vec3(0.0, -v.z, v.y);
}

// Quaternion functions from https://github.com/opengl-tutorials/ogl/blob/master/common/quaternion_utils.hpp
// Returns a quaternion such that q*start = dest
quat rotationBetweenVectors(vec3 start, vec3 dest){
    start = normalize(start);
    dest = normalize(dest);

    float cosTheta = dot(start, dest);
    vec3 rotationAxis;

    if (cosTheta < -1 + CLOSE_TO_0){
        // special case when vectors in opposite directions :
        // there is no "ideal" rotation axis
        // So guess one; any will do as long as it's perpendicular to start
        // This implementation favors a rotation around the Up axis,
        // since it's often what you want to do.
        /*
        rotationAxis = cross(VEC3_X, start);
        if (length2(rotationAxis) < 0.01 ) // bad luck, they were parallel, try again!
            rotationAxis = cross(VEC3_Y, start);

        rotationAxis = normalize(rotationAxis);
         */

        rotationAxis = normalize(orthogonal(start)); // Orthogonal is faster than cross
        return angleAxis(glm::radians(180.0f), rotationAxis);
    }

    // Implementation from Stan Melax's Game Programming Gems 1 article - avoids slow acosf()
    rotationAxis = cross(start, dest);

    float s = sqrtf((1 + cosTheta) * 2);
    float invs = 1 / s;

    return quat(
            s * 0.5f,
            rotationAxis.x * invs,
            rotationAxis.y * invs,
            rotationAxis.z * invs
    );
}

// Returns a quaternion that will make your object looking towards 'direction'.
// Similar to RotationBetweenVectors, but also controls the vertical orientation.
// Beware, the first parameter is a direction, not the target point!
quat lookAt(vec3 direction, vec3 desiredUp){
    if (length2(direction) < CLOSER_TO_0) {
        return quat();
    }

    // Recompute desiredUp so that it's perpendicular to the direction
    // You can skip that part if you really want to force desiredUp
    vec3 right = cross(direction, desiredUp);
    if(length2(right) > CLOSER_TO_0) {
        desiredUp = cross(right, direction);
    } else { // Direction and desiredUp are parallel
        // TODO: consider whether opposite directions need a 180 degree rotation
        return quat();
    }

    // Find the rotation between the front of the object and the desired direction
    quat rot1 = rotationBetweenVectors(WORLD_FRONT, direction);

    // Because of the 1st rotation, the up is probably completely screwed up.
    // Find the rotation between the "up" of the rotated object, and the desired up
    vec3 newUp = rot1 * WORLD_UP;
    quat rot2 = rotationBetweenVectors(newUp, desiredUp);

    // Apply them
    return rot2 * rot1; // remember, in reverse order.
}
