#ifndef DRAWABLE_H
#define DRAWABLE_H

#include <vector>

struct geometry {
    std::vector<glm::vec3>* vertices;
    std::vector<GLushort>* indices;
};

class Drawable {
public:
    virtual geometry genGeometry() = 0;
};

#endif //DRAWABLE_H
