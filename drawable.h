#ifndef DRAWABLE_H
#define DRAWABLE_H

#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>

struct vertex {
    GLfloat position[3];
    GLfloat normal[3];
};

struct geometry {
    geometry();

    std::vector<vertex>* vertices;
    std::vector<GLushort>* indices;

    void free() {
        delete vertices;
        delete indices;
    }
};

class Drawable {
public:
    virtual geometry genGeometry() = 0;
};

#endif //DRAWABLE_H
