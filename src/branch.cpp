#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "branch.h"
#include "consts.h"
#include "math_utils.h"

using std::vector;
using glm::vec3;
using glm::quat;

constexpr unsigned int DEFAULT_NUM_PTS = 8;

float defaultCrectionScale(float growthPct, float distAlongCurve) {
    return growthPct * powf(1.0f - distAlongCurve, 0.3f);
}

crection makeRegularCrection(unsigned int numPts) {
    crection ret;
    ret.reserve(numPts);

    float inc = 2 * PI_F / numPts;
    float angle = 0.0;

    for (int i = 0; i < numPts; i++) {
        ret.emplace_back(vec3{cosf(angle), sinf(angle), 0.0f});

        angle += inc;
    }

    return ret;
}

Branch::Branch(Bezier b, float scale, vec3 up)
        : numPts{DEFAULT_NUM_PTS},
        crossSection{makeRegularCrection(DEFAULT_NUM_PTS)},
        thicknessScalar{scale},
        bezier{std::move(b)},
        getCrectionScale{defaultCrectionScale},
        up{up} {}

void Branch::fillFace(vector<GLushort>* indices, GLushort firstIndex, bool reverse) const {
    if (!reverse) {
        for (GLushort i = firstIndex; i < firstIndex + numPts - 2; i++) {
            indices->push_back(firstIndex);
            indices->push_back(i + 1);
            indices->push_back(i + 2);
        }
    } else {
        for (GLushort i = firstIndex + numPts - 1; i > firstIndex + 1; i--) {
            indices->push_back(firstIndex);
            indices->push_back(i);
            indices->push_back(i - 1);
        }
    }
}

// Returns the next available index in buf
void Branch::fillExtrudedSquares(vector<GLushort>* indices, GLushort currentExtrusion) const {
    GLushort start = numPts * currentExtrusion;
    GLushort p = start; // Current point
    GLushort c = start + numPts; // Corresponding point on next crection
    GLushort n = start + 1; // Next point

    // Index all but one square - the last loops around to start
    for (; p < start + numPts - 1; p++, c++, n++) {
        indices->push_back(p);
        indices->push_back(c);
        indices->push_back(n);
        indices->push_back(n);
        indices->push_back(c);
        indices->push_back(c + 1);
    }

    // Index the final square
    indices->push_back(p);
    indices->push_back(c);
    indices->push_back(start);
    indices->push_back(start);
    indices->push_back(c);
    indices->push_back(p + 1);
}

void Branch::genCrection(geometry& g, float scale, vec3 center, quat rotate) const {
    for (auto const& v : crossSection) {
        // n has the un-normalized direction of the vertex compared to world center
        // n + center is the scaled/rotated/translated point
        vec3 n = rotate * v * scale;

        g.vertices->emplace_back(vertex{n.x + center.x, n.y + center.y, n.z + center.z, n.x, n.y, n.z});
    }
}

geometry Branch::genGeometry() const {
    geometry ret;

    // Return empty vectors if the growth percent is low enough
    if(growthPercent < 0.00001f)
        return ret;

    Bezier subcurve = bezier.subCurve(growthPercent);

    vector<vec3> b_verts = subcurve.getVertices();
    vector<vec3> b_dirs = subcurve.getDirections();
    vector<float> b_dists = subcurve.getDistances();

    auto numSections = (unsigned int) b_verts.size();

    ret.vertices->reserve(numPts * numSections);

    // Generate vertices
    for (int i = 0; i < numSections; ++i) {
        // The direction of the bezier at the center of this cross section is the
        // desired "front" for the face, so lookAt(direction, up) is the rotation quaternion
        quat rotQuat = lookAt(b_dirs[i], up);
        genCrection(ret, thicknessScalar * getCrectionScale(growthPercent, b_dists[i]), b_verts[i], rotQuat);
    }

    // Generate indices
    ret.indices->reserve(6 * numPts * numSections - 12);

    //fillFace(ret.indices, 0, false);

    for (GLushort i = 0; i < numSections - 1; i++) {
        fillExtrudedSquares(ret.indices, i);
    }

    //fillFace(ret.indices, numPts * (numSections - 1), true);

    return ret;
}

void Branch::setCrection(const crection& cr) {
    crossSection = cr;
    numPts = cr.size();
}

// b should be passed by const reference if Bezier is ever changed to reuse storage
// As it is, there is a Bezier deallocation either way
void Branch::setBezier(Bezier b) {
    bezier = std::move(b);
}

void Branch::setGrowth(float percent) {
    // Make sure we don't grow over 1.0f
    if(percent > 1.0f)
        growthPercent = 1.0f;
    else
        growthPercent = percent;
}

void Branch::setCrectionScaleFunc(crectionScaleFunc f) {
    getCrectionScale = std::move(f);
}


