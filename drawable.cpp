#include "drawable.h"

geometry::geometry()
        : vertices{new std::vector<vertex>()}, indices{new std::vector<GLushort>} {
}

void Drawable::setID(drawable_id newID) {
    id = newID;
}

drawable_id Drawable::getID() {
    return id;
}
