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
void buildTree() {
    tree->build();
}

// EXPOSED
// Branch CP Generation
em_wrapper<refBranch> branchCurveWrapper;

void setBranchCurveFunc(emscripten::val f) {
    branchCurveWrapper.setUpdate(f);
    tree->setBranchGenerator(branchCurveWrapper.getCallback());
}

void setBranchCurve(refBranch b) {
    branchCurveWrapper.set(b);
}

// Times to Branch
em_wrapper<int> timesToBranchWrapper;

void setTimesToBranchFunc(emscripten::val f) {
    timesToBranchWrapper.setUpdate(f);
    tree->setTimesToBranchFunc(timesToBranchWrapper.getCallback());
}

void setTimesToBranch(int n) {
    timesToBranchWrapper.set(n);
}

// Num Branches
em_wrapper<int> numBranchesWrapper;

void setNumBranchesFunc(emscripten::val f) {
    numBranchesWrapper.setUpdate(f);
    tree->setNumBranchesFunc(numBranchesWrapper.getCallback());
}

void setNumBranches(int n) {
    numBranchesWrapper.set(n);
}


EMSCRIPTEN_BINDINGS(my_module) {
        emscripten::function("initLogger", &initLogger);
        emscripten::function("buildTree", &buildTree);

        // Branch generation
        emscripten::function("setBranchCurveFunc", &setBranchCurveFunc);
        emscripten::function("setBranchCurve", &setBranchCurve);

        // Furcation variables
        emscripten::function("setTimesToBranchFunc", &setTimesToBranchFunc);
        emscripten::function("setTimesToBranch", &setTimesToBranch);

        emscripten::function("setNumBranchesFunc", &setNumBranchesFunc);
        emscripten::function("setNumBranches", &setNumBranches);

        emscripten::value_array<refBranch>("refBranch")
        .element(&refBranch::x1).element(&refBranch::y1).element(&refBranch::z1)
        .element(&refBranch::x2).element(&refBranch::y2).element(&refBranch::z2)
        .element(&refBranch::x3).element(&refBranch::y3).element(&refBranch::z3)
        ;
}
