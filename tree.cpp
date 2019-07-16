#include <iostream>

#include "tree.h"
#include "consts.h"
#include "math_utils.h"

#define DEFAULT_NUMPTS 8
#define BRANCH_SCALE 2.0f

using glm::vec3;
using glm::quat;

struct refBranch {
    vec3 c1, c2, c3;
};

Tree::Tree(Bezier trunk, unsigned int ticks, batchdrawer& drawer)
: lastTick{ticks}, drawer{drawer} {
    static refBranch ref{
            .c1 = vec3{0.3f, 0.0f, 0.0f} * BRANCH_SCALE,
            .c2 = vec3{0.8f, 0.3f, 0.0f} * BRANCH_SCALE,
            .c3 = vec3{0.9f, 0.7f, 0.0f} * BRANCH_SCALE,
    };

    curves.push_back(trunk);

    addBranch(trunk, 0.2f, 0, ticks, 1.0f / ticks);

    float branchPct = 0.7f;
    furcate(3, 3, branchPct, ref, 0.61f, trunk.pointAt(branchPct), 0, ticks);

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

void Tree::furcate(int timesToBranch, int numBranches, float pctAlong, const refBranch& ref, float scale, posAndDir p,
                   unsigned int pStartTick, unsigned int pTicks) {
    if (timesToBranch < 1) return;

    float inc = 2.0f * PI_F / numBranches;
    float angle = 0.0f;
    float newPct = pctAlong + (1.0f - pctAlong) / 3;

    // TODO: a = 0 roll is random due to noise of rotationBetweenVectors; use lookAt?
    for (int i = 0; i < numBranches; i++) {
        quat rot = rotationBetweenVectors(WORLD_UP, p.direction);
        rot = angleAxis(angle, p.direction) * rot;

        vec3 newc1 = rot * (ref.c1 * scale) + p.position;
        vec3 newc2 = rot * (ref.c2 * scale) + p.position;
        vec3 newc3 = rot * (ref.c3 * scale) + p.position;

        auto curve = Bezier{p.position, newc1, newc2, newc3, BEZIER_FLAT};

        auto newTicks = (unsigned int)(pTicks * 0.8f);
        if(newTicks < 1) newTicks = 1;
        unsigned int newStartTick = pStartTick + (unsigned int)(pTicks * pctAlong);
        unsigned int newEndTick = newStartTick + newTicks;

        addBranch(curve, 0.15f * scale, newStartTick, newEndTick, 1.0f / newTicks);

        lastTick = std::max(lastTick, newEndTick);

        angle += inc;

        furcate(timesToBranch - 1, numBranches, newPct, ref, scale * scale, curve.pointAt(newPct), newStartTick, newTicks);
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

