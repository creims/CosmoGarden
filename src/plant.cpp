#include "plant.h"
#include "consts.h"
#include "math_utils.h"

using glm::vec3;
using glm::quat;

Plant::Plant(plantRoot const& root) {
    build(root);
}

void Plant::setTrunk(plantRoot const& root) {
    build(root);
}

// Construct branches from a branchTree
void Plant::build(plantRoot const& root) {
    branches.clear();
    currTicks = 0;
    totalTicks = 0;

    for(auto const& branch : root.trunks) {
        addBranch(branch, posAndDir{VEC3_ORIGIN, WORLD_UP}, 0);
    }
}

void Plant::addBranch(branchDescription const& desc, posAndDir const& furcationPoint, unsigned int const branchStartTick) {
    unsigned int endTick = branchStartTick + desc.ticksToGrow;
    Branch b = makeBranch(desc.curve, desc.scale, desc.angle, furcationPoint.position, furcationPoint.direction);

    // Register the getCrectionScale function if it exists on the description
    if(desc.getCrectionScale != nullptr) {
        b.setCrectionScaleFunc(desc.getCrectionScale);
    }

    if(desc.crossSection.size() > 1) {
        b.setCrection(desc.crossSection);
    }

    branches.emplace_back(branchWrapper{(int)branches.size(), b, branchStartTick, endTick, desc.ticksToGrow});

    for (auto const& child : desc.children) {
        posAndDir childFurcationPoint = b.getBezier().pointAt(child.startRatio);
        unsigned int childStartTick = branchStartTick + (unsigned int)(child.ticksToGrow * child.startRatio);
        addBranch(child, childFurcationPoint, childStartTick);
    }

    if(endTick > totalTicks) {
        totalTicks = endTick;
    }
}

Branch Plant::makeBranch(refBranch const& ref, float const scale, float const angle, vec3 const& position, vec3 const& direction) {
    // TODO: a = 0 roll is random due to noise of rotationBetweenVectors; use lookAt?
    quat rot = rotationBetweenVectors(WORLD_UP, direction);
    rot = angleAxis(glm::radians(angle), direction) * rot;

    vec3 c0 = rot * vec3(ref.x0, ref.y0, ref.z0) * scale + position;
    vec3 c1 = rot * vec3(ref.x1, ref.y1, ref.z1) * scale + position;
    vec3 c2 = rot * vec3(ref.x2, ref.y2, ref.z2) * scale + position;
    vec3 c3 = rot * vec3(ref.x3, ref.y3, ref.z3) * scale + position;

    auto curve = Bezier{c0, c1, c2, c3, BEZIER_FLAT};
    return Branch{curve, scale, direction};
}

branch_id Plant::advance(unsigned int ticks) {
    unsigned int prevTicks = currTicks;
    currTicks += ticks;
    branch_id id = INVALID_BRANCH_ID;
    for(branch_id i = 0; i < branches.size(); i++) {
        branchWrapper& branch = branches[i];

        // Skip update if the branch hasn't started or is completely grown
        if(currTicks < branch.startTick || prevTicks > branch.endTick) {
            continue;
        }

        if(id == INVALID_BRANCH_ID) {
            id = i;
        }

        auto growthRatio = (float)(currTicks - branch.startTick) / branch.ticksToGrow;
        branch.branch.setGrowth(growthRatio);
    }

    return id;
}

std::vector<branchWrapper>& Plant::getBranches() {
    return branches;
}
