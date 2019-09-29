#include <iostream>

#include <memory>
#include <math.h>
#include <SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stddef.h>

#include "common.h"
#include "gfx/shader_utils.h"
#include "tree.h"
#include "consts.h"
#include "Arcball.h"
#include "Camera.h"
#include "batchdrawer.h"
#include "gfx/SolidColorMaterial.h"
#include "Scene.h"

// TODO: rename this file (what is common...?) and refactor to make it less monolothic/disorganized/awful

using glm::vec3;
using glm::mat4;
using glm::quat;

static bool quit = false;

// TODO: Fix winding weirdness exhibited by this test branch
plantRoot initTestTree() {
    branchDescription trunk{};
    trunk.curve = refBranch{
            0.5f, -3.0f, -0.1f,
            0.2f, -1.0f, 0.1f,
            -0.5f, 2.0f, 0.3f,
            0.0f, 3.5f, 0.1f
    };

    trunk.ticksToGrow = 0;
    plantRoot root{};
    root.trunks.push_back(trunk);
    return root;
}

plantRoot initDefaultTree() {
    branchDescription trunk{};
    trunk.curve = refBranch{
            0.5f, 0.0f, 0.1f,
            0.2f, 2.0f, 0.1f,
            0.5f, 5.0f, 0.3f,
            0.0f, 6.5f, 0.1f
    };

    crectionScaleFunc def = [](float growthPct, float distAlongCurve) {
        return 0.4 * growthPct * powf(1.0 - distAlongCurve, 0.3f);
    };

    trunk.getCrectionScale = def;

    branchDescription branch1{};
    branch1.crossSection.assign(
            {vec3{-1.0f, 0.0f, 0.0f}, vec3{0.0f, 0.5f, 0.0f}, vec3{1.0f, 0.0f, 0.0f}, vec3{0.0f, -0.5f, 0.0f}});
    branch1.scale = 0.61f;
    branch1.getCrectionScale = def;
    branchDescription branch2 = branch1;
    branch2.angle = 120.0f;
    branchDescription branch3 = branch1;
    branch3.angle = 240.0f;
    trunk.children.push_back(branch1);
    trunk.children.push_back(branch2);
    trunk.children.push_back(branch3);
    trunk.children[1].children.push_back(branch1);
    trunk.children[1].children.push_back(branch2);
    trunk.children[1].children.push_back(branch3);

    plantRoot root{};
    root.trunks.push_back(trunk);
    return root;
}

Scene& initScene(int w, int h, SDL_Window* window) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Our geometry generation winds clockwise, while the default is anti-clockwise
    glFrontFace(GL_CW);

    auto scene = new Scene(window);
    scene->resizeWindow(w, h);

    auto root = initDefaultTree();
    scene->buildTree(root);

    return *scene;
}

void swapCameraMode(Scene& scene) {
    scene.toggleCamera();
    if (scene.isArcballCamera()) {
        SDL_SetRelativeMouseMode(SDL_FALSE);
    } else {
        SDL_SetRelativeMouseMode(SDL_TRUE);
    }
}

void updateTicks(Scene& scene) {
    static float frac{0};
    static unsigned int lastTicks{0};

    unsigned int diff = SDL_GetTicks() - lastTicks;
    lastTicks += diff;

    frac += diff * TICKS_PER_MS;
    diff = (unsigned int) frac;

    frac -= diff;
    scene.advanceTime(diff);
}

struct KeysPressed {
    bool w{false}, a{false}, s{false}, d{false};
};

void handleKeyboard(Scene& scene, KeysPressed const& keys) {
    constexpr float moveSpeed = 0.01f; // meters per ms
    static unsigned int lastTicks{0};

    if(scene.isArcballCamera()) {
        return;
    }

    unsigned int ticks = SDL_GetTicks();
    float magnitude = (float)(ticks - lastTicks) * moveSpeed; // milliseconds * m/ms
    lastTicks = ticks;

    if (keys.w) scene.cameraForward(magnitude);
    if (keys.s) scene.cameraBackward(magnitude);
    if (keys.a) scene.cameraLeft(magnitude);
    if (keys.d) scene.cameraRight(magnitude);
}

void handleMouseWheel(Scene& scene, float dy) {
    if (dy > 0) { // scroll up
        scene.cameraZoom(0.8);
    } else if (dy < 0) { // scroll down
        scene.cameraZoom(1.2);
    }
}

void handleMouseMotion(Scene& scene, SDL_MouseMotionEvent motion, bool mouseDown) {
    if (scene.isArcballCamera()) {
        scene.handleMouseDrag(motion.x, motion.y, mouseDown);
    } else {
        scene.handleMouseDrag(motion.xrel, motion.yrel, mouseDown);
    }
}

void update(Scene& scene) {
    SDL_Event e;
    static KeysPressed keys{};
    static bool mouseDown;

    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            quit = true;
        } else if (e.type == SDL_MOUSEBUTTONDOWN) {
            mouseDown = true;
            scene.handleMouseDown(e.button.x, e.button.y);
        } else if (e.type == SDL_MOUSEBUTTONUP) {
            mouseDown = false;
        } else if (e.type == SDL_MOUSEMOTION) {
            handleMouseMotion(scene, e.motion, mouseDown);
        } else if (e.type == SDL_MOUSEWHEEL) {
            handleMouseWheel(scene, e.wheel.y);
        } else if (e.type == SDL_WINDOWEVENT) {
            if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
                scene.resizeWindow(e.window.data1, e.window.data2);
            }
        } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_r:
                    swapCameraMode(scene);
                    break;
                case SDLK_ESCAPE:
                    scene.defaultCamera();
                    break;
                case SDLK_w:
                    keys.w = true;
                    break;
                case SDLK_s:
                    keys.s = true;
                    break;
                case SDLK_a:
                    keys.a = true;
                    break;
                case SDLK_d:
                    keys.d = true;
                    break;
            }
        } else if (e.type == SDL_KEYUP) {
            switch (e.key.keysym.sym) {
                case SDLK_w:
                    keys.w = false;
                    break;
                case SDLK_s:
                    keys.s = false;
                    break;
                case SDLK_a:
                    keys.a = false;
                    break;
                case SDLK_d:
                    keys.d = false;
                    break;
            }
        }
    }

    handleKeyboard(scene, keys);
    updateTicks(scene);
    scene.render();
}

bool shouldQuit() {
    return quit;
}
