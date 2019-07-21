#include <emscripten/bind.h>
#include <iostream>
#include <sstream>
#include <string>
#include <glm/glm.hpp>
#include "tree.h"

using glm::vec3;

struct RefBranchMemory {
    float x1, y1, z1, x2, y2, z2, x3, y3, z3;
};

struct RefBranchMemory mem;
emscripten::val errorHandler = emscripten::val::undefined();
emscripten::val logHandler = emscripten::val::undefined();

emscripten::val getBranchCPs = emscripten::val::undefined();

Tree* tree;

void setActiveTree(Tree* t) {
    tree = t;
}

void initLogger() {
    logHandler = emscripten::val::global("log");

    std::string msg = "Initialization successful.";
    logHandler(msg);
}

std::string memToStr() {
    std::ostringstream ss;
    ss << mem.x1 << " " << mem.y1 << " " << mem.z1 << " "
    << mem.x2 << " " << mem.y2 << " " << mem.z2 << " "
    << mem.x3 << " " << mem.y3 << " " << mem.z3;
    return ss.str();
}

refBranch genBranch() {
    getBranchCPs();
    return refBranch {
        vec3{mem.x1, mem.y1, mem.z1},
        vec3{mem.x2, mem.y2, mem.z2},
        vec3{mem.x3, mem.y3, mem.z3}
        };
}

void setBranchCPGenerator(emscripten::val getCPs) {
    getBranchCPs = getCPs;
    tree->setBranchGenerator(genBranch);
    tree->build();
}

void setCPs(RefBranchMemory m) {
    mem = m;
}

EMSCRIPTEN_BINDINGS(my_module) {
        emscripten::function("initLogger", &initLogger);
        emscripten::function("setBranchCPGenerator", &setBranchCPGenerator);
        emscripten::function("setCPMemory", &setCPs);

        emscripten::value_array<RefBranchMemory>("RefBranchMemory")
        .element(&RefBranchMemory::x1).element(&RefBranchMemory::y1).element(&RefBranchMemory::z1)
        .element(&RefBranchMemory::x2).element(&RefBranchMemory::y2).element(&RefBranchMemory::z2)
        .element(&RefBranchMemory::x3).element(&RefBranchMemory::y3).element(&RefBranchMemory::z3)
        ;
}
