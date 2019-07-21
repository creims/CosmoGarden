#ifndef BATCHDRAWER_H
#define BATCHDRAWER_H

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <GL/glew.h>

#include "drawable.h"

typedef unsigned int object_id;
constexpr object_id INVALID_OBJECT_ID = 0;

class batchdrawer {
public:
    batchdrawer();

    void clear();

    object_id registerObject(Drawable& d);
    void updateObject(object_id id);

    void prepareToDraw();
    void draw();

    void setBufferDirty();

private:
    struct ObjectInfo {
        explicit ObjectInfo(Drawable& d) : object{d} {};
        Drawable& object;
        size_t vertexStart{0};
        size_t indexStart{0};
    };

    std::map<object_id, ObjectInfo> registeredObjects;

    bool bufferDirty{true};

    unsigned int start_vertices{0};
    size_t size_vbuf{0}, size_ibuf{0}, start_vbuf{0}, start_ibuf{0};

    GLuint ibo{0}, vbo{0};

    void restoreTo(ObjectInfo& oi);
};

#endif //BATCHDRAWER_H
