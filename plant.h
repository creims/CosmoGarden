#ifndef PLANT_H
#define PLANT_H

#include <vector>

#include "branch.h"

typedef int branch_id;
constexpr branch_id INVALID_BRANCH_ID = -1;

struct refBranch {
    refBranch() = default;
    refBranch(float x0, float y0, float z0, float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3)
    : x0{x0}, y0{y0}, z0{z0},  x1{x1}, y1{y1}, z1{z1}, x2{x2}, y2{y2}, z2{z2},  x3{x3}, y3{y3}, z3{z3} {};
    refBranch(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3)
            : x1{x1}, y1{y1}, z1{z1}, x2{x2}, y2{y2}, z2{z2},  x3{x3}, y3{y3}, z3{z3} {};
    float x0{0.0f}, y0{0.0f}, z0{0.0f},
    x1{1.0f}, y1{1.0f}, z1{1.0f},
    x2{2.0f}, y2{2.0f}, z2{2.0f},
    x3{3.0f}, y3{3.0f}, z3{3.0f};
};

struct branchDescription {
    refBranch curve{-0.5f, 1.0f, 0.5f, 0.5f, 2.0f, 0.8f, -2.0f, 3.0f, 1.8f};
    float startRatio{0.6f};
    float scale{1.0f};
    float angle{0.0f};
    unsigned int ticksToGrow{50};
    std::vector<branchDescription> children{};
};

// TODO: just add tick info to branches instead of having a wrapper?
struct branchWrapper {
    branch_id id{0};
    Branch branch{};
    unsigned int startTick{0}, endTick{0}, ticksToGrow{0};
};

class Plant {
public:
    Plant() = default;
    explicit Plant(branchDescription const& trunk);
    void setTrunk(branchDescription const& trunk);

    // Returns the id of the first changed branch
    branch_id advance(unsigned int ticks);

    std::vector<branchWrapper>& getBranches();
private:
    std::vector<branchWrapper> branches{};
    unsigned int currTicks{0}, totalTicks{0};

    void build(branchDescription const& trunk);
    void addBranch(branchDescription const& desc, posAndDir const& parentPD, unsigned int branchStartTick);
    Branch makeBranch(refBranch const& ref, float scale, float angle, glm::vec3 const& position, glm::vec3 const& direction);
};


#endif //PLANT_H
