#ifndef COMMON_H
#define COMMON_H

#include "tree.h"

bool cInit(int w, int h, SDL_Window* window);
void cClose();
bool initResources();

void update();
bool shouldQuit();

Tree* getSceneTree();

#endif //COMMON_H
