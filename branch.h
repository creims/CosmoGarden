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

    void setBezier(Bezier& b);
    void setGrowth(float percent);

    geometry genGeometry() override;

private:
    Bezier bezier;
    unsigned int numPts{0};
    unsigned int numSections{0};
    float globalScale{1.0f};
    float growthPercent{1.0f};

    void genCrection(std::vector<glm::vec3>* vertices, float scale, glm::vec3 center, glm::quat rotate);
    void fillFace(std::vector<GLushort>* indices, GLushort firstIndex, bool reverse);
    void fillExtrudedSquares(std::vector<GLushort>* indices, GLushort currentExtrusion);

    float getScale(float growthPct, float distAlongCurve);
};

#endif //BRANCH_H
