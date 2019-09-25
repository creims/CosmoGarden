#include <iostream>
#include <algorithm>

#include "tree.h"
#include "consts.h"
#include "math_utils.h"

using glm::vec3;
using glm::quat;

Tree::Tree(const plantRoot& root, batchdrawer& drawer)
        : drawer{drawer} {
    buildFromRoot(root);
}

/*
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
}*/

// TODO: update to use ticks (or make a tick version in addition)
void Tree::setGrowth(float growthPercent) {
    currTicks = std::min(lastTick, (unsigned int) (lastTick * growthPercent));
}

void Tree::advance(unsigned int numTicks) {
    if (numTicks < 1)
        return;

    currTicks = std::min(lastTick, currTicks + numTicks);
    drawable_id id = plant->advance(numTicks);
    drawer.updateFrom(id);
}

void Tree::buildFromRoot(const plantRoot& root) {
    drawer.clear();
    plant = std::make_unique<Plant>(root);
    std::vector<branchWrapper>& bs = plant->getBranches();

    // Because the drawer updates from the first branch which has changed,
    // order the branches by startTick before registering them or they will not
    // draw at the proper times
    std::sort(bs.begin(), bs.end(),
              [](branchWrapper const& a, branchWrapper const& b) {
                  return a.startTick < b.startTick;
              });

    for (auto& d : bs) {
        drawer.registerObject(d.branch);
    }
}


