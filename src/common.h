#ifndef COMMON_H
#define COMMON_H

#include "tree.h"
#include "Scene.h"

Scene& initScene(int w, int h, SDL_Window* window);

void update(Scene& scene);
bool shouldQuit();

#endif //COMMON_H
