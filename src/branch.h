#ifndef BRANCH_H
#define BRANCH_H

#include <vector>
#include <functional>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "bezier.h"
#include "drawable.h"
#include "consts.h"

typedef std::vector<glm::vec3> crection;
typedef std::function<float(float, float)> crectionScaleFunc;

class Branch : public Drawable {
public:
    Branch() = default;
    Branch(Bezier b, float scale, glm::vec3 up);

    void setCrection(const crection& crossSection);
    void setBezier(Bezier b);
    Bezier const& getBezier() const { return bezier; };
    void setGrowth(float percent);

    void setCrectionScaleFunc(crectionScaleFunc f);

    geometry genGeometry() const override;

private:
    Bezier bezier;
    crection crossSection;
    unsigned int numPts{0};
    float thicknessScalar{1.0f};
    float growthPercent{0.0f};
    glm::vec3 up{WORLD_UP};

    void genCrection(geometry& g, float scale, glm::vec3 center, glm::quat rotate) const;
    void fillFace(std::vector<GLushort>* indices, GLushort firstIndex, bool reverse) const;
    void fillExtrudedSquares(std::vector<GLushort>* indices, GLushort currentExtrusion) const;

    crectionScaleFunc getCrectionScale;
};

#endif //BRANCH_H
