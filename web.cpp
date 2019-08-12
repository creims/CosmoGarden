#include <emscripten/bind.h>
#include <iostream>
#include <sstream>
#include <string>
#include <glm/glm.hpp>
#include "tree.h"
#include "em_wrapper.h"

using glm::vec3;

emscripten::val logHandler = emscripten::val::undefined();
emscripten::val branchScaleHandler = emscripten::val::undefined();

Tree* tree;

void setActiveTree(Tree* t) {
    tree = t;
}

void initAll() {
    logHandler = emscripten::val::global("log");
    branchScaleHandler = emscripten::val::global("getBranchScale");

    std::string msg = "Initialization successful.";
    logHandler(msg);
}

float getBranchScalar(unsigned int id, float growthRatio, float distAlongCurve) {
    float scalar = branchScaleHandler(id, growthRatio, distAlongCurve).as<float>();
    return scalar;
}

void registerCrectionScaleFuncs(branchDescription& branch) {
    branch.getCrectionScale = [id = branch.id](float growthRatio, float distAlongCurve) -> float {
            return getBranchScalar(id, growthRatio, distAlongCurve);
    };

    for(auto& child : branch.children) {
        registerCrectionScaleFuncs(child);
    }
}

void buildPlant(branchDescription& trunk) {
    registerCrectionScaleFuncs(trunk);
    tree->buildFromTrunk(trunk);
}

std::vector<branchDescription> makeBranchVector() {
    return std::vector<branchDescription>{};
}

EMSCRIPTEN_BINDINGS(my_module) {
        emscripten::function("initAll", &initAll);
        emscripten::function("buildPlant", &buildPlant);
        emscripten::function("makeBranchVector", &makeBranchVector);

        emscripten::value_array<refBranch>("refBranch")
        .element(&refBranch::x0).element(&refBranch::y0).element(&refBranch::z0)
        .element(&refBranch::x1).element(&refBranch::y1).element(&refBranch::z1)
        .element(&refBranch::x2).element(&refBranch::y2).element(&refBranch::z2)
        .element(&refBranch::x3).element(&refBranch::y3).element(&refBranch::z3)
        ;

        emscripten::register_vector<branchDescription>("vector<branchDescription>");

        emscripten::value_object<branchDescription>("branchDescription")
        .field("id", &branchDescription::id)
        .field("curve", &branchDescription::curve)
        .field("startRatio", &branchDescription::startRatio)
        .field("scale", &branchDescription::scale)
        .field("angle", &branchDescription::angle)
        .field("ticksToGrow", &branchDescription::ticksToGrow)
        .field("children", &branchDescription::children)
        ;
}
