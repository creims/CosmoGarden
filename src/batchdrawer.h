#ifndef BATCHDRAWER_H
#define BATCHDRAWER_H

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <GL/glew.h>

#include "drawable.h"

class batchdrawer {
public:
    batchdrawer();

    void clear();

    void registerObject(Drawable& d);
    void updateFrom(drawable_id id);

    void prepareToDraw();
    void draw();

private:
    struct DrawableInfo {
        explicit DrawableInfo(Drawable& d) : drawable{d} {};
        Drawable& drawable;
        size_t vertexStart{0};
        size_t indexStart{0};
    };

    std::vector<DrawableInfo> registeredObjects;

    unsigned int start_vertices{0};
    size_t size_vbuf{0}, size_ibuf{0}, start_vbuf{0}, start_ibuf{0};

    GLuint ibo{0}, vbo{0};

    void restoreTo(DrawableInfo& oi);
};

#endif //BATCHDRAWER_H
