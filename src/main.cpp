#include <stdlib.h>
#include <GL/glew.h>
#include "SDL.h"
#include <stdio.h>
#include <iostream>
#include "common.h"

#ifdef EMSCRIPTEN
#   include "emscripten.h"
#   include "emscripten/html5.h"
#   include "../web.cpp"
#endif

static SDL_Window* window;
static SDL_GLContext glContext;

extern "C" int main(int argc, char** argv) {
    int w = 800;
    int h = 800;
#ifdef EMSCRIPTEN
    SDL_Renderer* renderer = NULL;
    emscripten_get_canvas_element_size("#canvas", &w, &h);
    SDL_CreateWindowAndRenderer(w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE, &window, &renderer);
#else
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL init error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    window = SDL_CreateWindow("CosmoGarden", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    glContext = SDL_GL_CreateContext(window);
#endif

    GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK) {
        printf("GLEW init error: %s\n", glewGetErrorString(glewStatus));
        return 1;
    }

    if(!cInit(w, h, window)) {
        return 1;
    }

    if(!initResources()) {
        return 1;
    }

#ifdef EMSCRIPTEN
    setActiveTree(getSceneTree());

    disableKeyboard();

    emscripten_set_main_loop(update, 0, 1);
#else

    while(!shouldQuit()) {
        update();
    }
    cClose();

    return 0;
#endif
}
