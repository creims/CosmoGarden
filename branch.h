#ifndef BRANCH_H
#define BRANCH_H

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "bezier.h"
#include "drawable.h"

class Branch : public Drawable {
public:
    Branch() = default;
    Branch(unsigned int numPts, Bezier b, float scale);

    void setBezier(Bezier const& b);
    Bezier const& getBezier() { return bezier; };
    void setGrowth(float percent);

    geometry genGeometry() override;

private:
    Bezier bezier;
    unsigned int numPts{0};
    float thicknessScalar{1.0f};
    float growthPercent{0.0f};

    void genCrection(geometry& g, float scale, glm::vec3 center, glm::quat rotate);
    void fillFace(std::vector<GLushort>* indices, GLushort firstIndex, bool reverse);
    void fillExtrudedSquares(std::vector<GLushort>* indices, GLushort currentExtrusion);

    float getCrectionScale(float growthPct, float distAlongCurve);
};

#endif //BRANCH_H
