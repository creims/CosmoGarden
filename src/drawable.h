#ifndef DRAWABLE_H
#define DRAWABLE_H

#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>

typedef int drawable_id;
constexpr drawable_id INVALID_DRAWABLE_ID = -1;

struct vertex {
    GLfloat position[3];
    GLfloat normal[3];
};

// TODO: instead of passing these around, have drawables generate into vectors
struct geometry {
    geometry();

    ~geometry() {
        delete vertices;
        delete indices;
    }

    std::vector<vertex>* vertices;
    std::vector<GLushort>* indices;
};

class Drawable {
public:
    virtual geometry genGeometry() = 0;
    void setID(drawable_id newID);
    drawable_id getID();
private:
    drawable_id id{INVALID_DRAWABLE_ID};
};

#endif //DRAWABLE_H
