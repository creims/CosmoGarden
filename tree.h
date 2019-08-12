#ifndef TREE_H
#define TREE_H

#include <functional>
#include <memory>
#include "batchdrawer.h"
#include "bezier.h"
#include "branch.h"
#include "plant.h"

class Tree {
public:
    Tree(branchDescription const& trunk, batchdrawer& drawer);

    void buildFromTrunk(branchDescription const& trunk);
    void setGrowth(float growthPercent);
    void advance(unsigned int numTicks);

private:
    unsigned int currTicks{0}, lastTick{0};
    batchdrawer& drawer;
    std::unique_ptr<Plant> plant;
};


#endif //TREE_H
