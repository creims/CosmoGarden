#include <emscripten/bind.h>
#include <iostream>
#include <sstream>
#include <string>
#include <glm/glm.hpp>
#include "tree.h"
#include "em_wrapper.h"

using glm::vec3;

emscripten::val errorHandler = emscripten::val::undefined();
emscripten::val logHandler = emscripten::val::undefined();

Tree* tree;

void setActiveTree(Tree* t) {
    tree = t;
}

void initLogger() {
    logHandler = emscripten::val::global("log");

    std::string msg = "Initialization successful.";
    logHandler(msg);
}

// GENERAL
void buildPlant(branchDescription const& trunk) {
    tree->buildFromTrunk(trunk);
}

/*
struct branchDescription {
    refBranch curve{-0.5f, 1.0f, 0.5f, 0.5f, 2.0f, 0.8f, -2.0f, 3.0f, 1.8f};
    float startRatio{0.6f};
    float scale{1.0f};
    float angle{0.0f};
    unsigned int ticksToGrow{50};
    std::vector<branchDescription> children{};
};
*/

void logBranch(branchDescription const& b) {
    logHandler(b.children.size());
    for(auto& child : b.children) {
        logBranch(child);
    }
}

std::vector<branchDescription> makeBranchVector() {
    return std::vector<branchDescription>{};
}

EMSCRIPTEN_BINDINGS(my_module) {
        emscripten::function("initLogger", &initLogger);
        emscripten::function("buildPlant", &buildPlant);
        emscripten::function("logBranch", &logBranch);
        emscripten::function("makeBranchVector", &makeBranchVector);

        emscripten::value_array<refBranch>("refBranch")
        .element(&refBranch::x0).element(&refBranch::y0).element(&refBranch::z0)
        .element(&refBranch::x1).element(&refBranch::y1).element(&refBranch::z1)
        .element(&refBranch::x2).element(&refBranch::y2).element(&refBranch::z2)
        .element(&refBranch::x3).element(&refBranch::y3).element(&refBranch::z3)
        ;

        emscripten::register_vector<branchDescription>("vector<branchDescription>");

        emscripten::value_object<branchDescription>("branchDescription")
        .field("curve", &branchDescription::curve)
        .field("startRatio", &branchDescription::startRatio)
        .field("scale", &branchDescription::scale)
        .field("angle", &branchDescription::angle)
        .field("ticksToGrow", &branchDescription::ticksToGrow)
        .field("children", &branchDescription::children)
        ;
}
