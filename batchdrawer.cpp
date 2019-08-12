#include "batchdrawer.h"
#include <iostream>

constexpr size_t MAX_BUF_SIZE = 1024 * 1024;

void checkError() {
    auto e = glGetError();
    if(e != GL_NO_ERROR) {
        std::cout << "Error: " << e;
    }
}

batchdrawer::batchdrawer()
{
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_BUF_SIZE, nullptr, GL_STREAM_DRAW);

    checkError();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_BUF_SIZE, nullptr, GL_STREAM_DRAW);

    checkError();
}

void batchdrawer::registerObject(Drawable& d) {
    drawable_id id = registeredObjects.size();
    d.setID(id);
    registeredObjects.emplace_back(DrawableInfo{d});
}

void batchdrawer::updateFrom(drawable_id id) {
    if(id < 0 || id >= registeredObjects.size()) {
        return;
    }

    restoreTo(registeredObjects[id]);

    while(id < registeredObjects.size()) {
        DrawableInfo& info = registeredObjects[id];

        geometry g = info.drawable.genGeometry();

        if (g.vertices->empty() || g.indices->empty()) {
            return;
        }

        // Check to see if the branch will fit in the buffer. If not, give an error
        // TODO: error handling
        size_t new_vbuf_size = size_vbuf + g.vertices->size() * sizeof(vertex);
        size_t new_ibuf_size = size_ibuf + g.indices->size() * sizeof(GLushort);
        if (new_vbuf_size > MAX_BUF_SIZE || new_ibuf_size > MAX_BUF_SIZE) {
            std::cout << "Overrunning the buffer.." << std::endl;
            return;
        }

        info.vertexStart = size_vbuf;
        info.indexStart = size_ibuf;

        size_vbuf = new_vbuf_size;
        size_ibuf = new_ibuf_size;

        // Increment the index values by the number of vertices before it in the buffer
        for (auto& index : *g.indices) {
            index += start_vertices;
        }

        start_vertices += g.vertices->size();

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, start_vbuf, size_vbuf - start_vbuf, g.vertices->data());

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, start_ibuf, size_ibuf - start_ibuf, g.indices->data());

        start_vbuf = size_vbuf;
        start_ibuf = size_ibuf;

        id++;
    }
}

void batchdrawer::prepareToDraw() {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
}

void batchdrawer::draw() {
    glDrawElements(GL_TRIANGLES, size_ibuf / sizeof(GLushort), GL_UNSIGNED_SHORT, nullptr);
}

void batchdrawer::restoreTo(DrawableInfo& oi) {
    start_vbuf = oi.vertexStart;
    start_ibuf = oi.indexStart;

    size_vbuf = start_vbuf;
    size_ibuf = start_ibuf;

    start_vertices = start_vbuf / sizeof(vertex);
}

void batchdrawer::clear() {
    start_vbuf = 0;
    start_ibuf = 0;
    size_vbuf = 0;
    size_ibuf = 0;
    start_vertices = 0;

    registeredObjects.clear();
}


