#include "drawable.h"

geometry::geometry()
        : vertices{new std::vector<vertex>()}, indices{new std::vector<GLushort>} {
}