#include <iostream>

#include "tree.h"
#include "consts.h"
#include "math_utils.h"

#define DEFAULT_NUMPTS 8

using glm::vec3;
using glm::quat;

refBranch defaultGenBranch() {
    static refBranch ref{
            0.6f, 0.0f, 0.0f,
            1.6f, 0.3f, 0.0f,
            1.8f, 1.4f, 0.0f,
    };

    return ref;
}

int defaultTimesToBranch() {
    return 3;
}

int defaultNumBranches() {
    return 3;
}

Tree::Tree(Bezier trunk, unsigned int ticks, batchdrawer& drawer)
: trunk{std::move(trunk)}, trunkTicks{ticks}, lastTick{ticks}, drawer{drawer} {
    genBranch = defaultGenBranch;
    getTimesToBranch = defaultTimesToBranch;
    getNumBranches = defaultNumBranches;

    build();
}

void Tree::build() {
    drawer.clear();
    curves.clear();
    branches.clear();
    branchData.clear();

    currTicks = 0;

    curves.push_back(trunk);

    addBranch(trunk, 0.2f, 0, trunkTicks, 1.0f / trunkTicks);

    float branchPct = 0.7f;
    furcate(getTimesToBranch(), branchPct, 0.61f, trunk.pointAt(branchPct), 0, trunkTicks);

    for(int i = 0; i < branches.size(); i++) {
        branchData[i].id = drawer.registerObject(branches[i]);
    }
}

void Tree::addBranch(Bezier& curve, float radius, unsigned int startTick, unsigned int endTick, float tickIncrement) {
    curves.push_back(curve);

    branches.emplace_back(Branch{DEFAULT_NUMPTS, curve, radius});

    // Register the trunk
    branchData.emplace_back(branchInfo{INVALID_OBJECT_ID, startTick, endTick, tickIncrement, false});
}

void Tree::furcate(int timesToBranch, float pctAlong, float scale, posAndDir p, unsigned int pStartTick, unsigned int pTicks) {
    int numBranches = getNumBranches();
    if (timesToBranch < 1 || numBranches < 1) return;

    float inc = 2.0f * PI_F / numBranches;
    float angle = 0.0f;
    float newPct = pctAlong + (1.0f - pctAlong) / 3;

    // TODO: a = 0 roll is random due to noise of rotationBetweenVectors; use lookAt?
    for (int i = 0; i < numBranches; i++) {
        refBranch ref = genBranch();
        quat rot = rotationBetweenVectors(WORLD_UP, p.direction);
        rot = angleAxis(angle, p.direction) * rot;

        vec3 newc1 = rot * vec3(ref.x1, ref.y1, ref.z1) * scale + p.position;
        vec3 newc2 = rot * vec3(ref.x2, ref.y2, ref.z2) * scale + p.position;
        vec3 newc3 = rot * vec3(ref.x3, ref.y3, ref.z3) * scale + p.position;

        auto curve = Bezier{p.position, newc1, newc2, newc3, BEZIER_FLAT};

        auto newTicks = (unsigned int)(pTicks * 0.8f);
        if(newTicks < 1) newTicks = 1;
        unsigned int newStartTick = pStartTick + (unsigned int)(pTicks * pctAlong);
        unsigned int newEndTick = newStartTick + newTicks;

        addBranch(curve, 0.15f * scale, newStartTick, newEndTick, 1.0f / newTicks);

        lastTick = std::max(lastTick, newEndTick);

        angle += inc;

        furcate(timesToBranch - 1, newPct, scale * scale, curve.pointAt(newPct), newStartTick, newTicks);
    }
}

// TODO: update to use ticks
void Tree::setGrowth(float growthPercent) {
    currTicks = std::min(lastTick, (unsigned int)(lastTick * growthPercent));
    updateBranches();
}

void Tree::advance(unsigned int numTicks) {
    if(numTicks < 1 || currTicks == lastTick)
        return;

    currTicks = std::min(lastTick, currTicks + numTicks);
    updateBranches();
}

void Tree::updateBranches() {
    drawer.setBufferDirty();
    for(int i = 0; i < curves.size(); i++) {
        branchInfo info = branchData[i];
        if(currTicks <= info.startTick || info.done)
            continue;

        if(currTicks > info.endTick)
            info.done = true;

        float pct = (currTicks - info.startTick) * info.increment;
        branches[i].setGrowth(pct);
        drawer.updateObject(info.id);
    }
}

void Tree::setBranchGenerator(refBranchFn gen) {
    genBranch = std::move(gen);
}

void Tree::setTimesToBranchFunc(intFn gen) {
    getTimesToBranch = std::move(gen);
}

void Tree::setNumBranchesFunc(intFn gen) {
    getNumBranches = std::move(gen);
}

