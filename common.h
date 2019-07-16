#ifndef COMMON_H
#define COMMON_H

bool cInit(int w, int h, SDL_Window* window);
void cClose();
bool initResources();

void update();
bool shouldQuit();

#endif //COMMON_H
