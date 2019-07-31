#ifndef TREE_H
#define TREE_H

#include <functional>
#include "batchdrawer.h"
#include "bezier.h"
#include "branch.h"

struct refBranch {
    float x1, y1, z1, x2, y2, z2, x3, y3, z3;
};

struct branchInfo {
    object_id id{INVALID_OBJECT_ID};
    unsigned int startTick{0}, endTick{0};
    float increment{0.0f};
    bool done{false};
};

typedef std::function<refBranch(void)> refBranchFn;
typedef std::function<int(void)> intFn;

class Tree {
public:
    Tree(Bezier trunk, unsigned int ticks, batchdrawer& drawer);

    void build();
    void setGrowth(float growthPercent);
    void advance(unsigned int numTicks);

    void setBranchGenerator(refBranchFn gen);
    void setTimesToBranchFunc(intFn gen);
    void setNumBranchesFunc(intFn gen);

private:
    unsigned int currTicks{0}, lastTick{0}, trunkTicks{0};
    batchdrawer& drawer;
    Bezier trunk;

    std::vector<Bezier> curves;
    std::vector<Branch> branches;
    std::vector<branchInfo> branchData;

    refBranchFn genBranch;
    intFn getTimesToBranch;
    intFn getNumBranches;

    void addBranch(Bezier& curve, float radius, unsigned int startTick, unsigned int endTick, float tickIncrement);
    void furcate(int timesToBranch, float pctAlong, float scale, posAndDir p, unsigned int pStartTick, unsigned int pTicks);
    void updateBranches();
};


#endif //TREE_H
