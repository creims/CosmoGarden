#ifndef TREE_H
#define TREE_H

#include "batchdrawer.h"
#include "bezier.h"
#include "branch.h"

struct refBranch;
struct branchInfo {
    object_id id{INVALID_OBJECT_ID};
    unsigned int startTick{0}, endTick{0};
    float increment{0.0f};
    bool done{false};
};

class Tree {
public:
    Tree(Bezier trunk, unsigned int ticks, batchdrawer& drawer);

    void setGrowth(float growthPercent);
    void advance(unsigned int numTicks);

private:
    unsigned int currTicks{0};
    unsigned int lastTick{0};
    batchdrawer& drawer;

    std::vector<Bezier> curves;
    std::vector<Branch> branches;
    std::vector<branchInfo> branchData;

    void addBranch(Bezier& curve, float radius, unsigned int startTick, unsigned int endTick, float tickIncrement);
    void furcate(int timesToBranch, int numBranches, float pctAlong, const refBranch& ref, float scale, posAndDir p,
                     unsigned int pStartTick, unsigned int pTicks);
    void updateBranches();
};


#endif //TREE_H
