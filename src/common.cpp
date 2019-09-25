#include <iostream>

#include <memory>
#include <math.h>
#include <SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stddef.h>

#include "common.h"
#include "shader_utils.h"
#include "tree.h"
#include "consts.h"
#include "Arcball.h"
#include "Camera.h"
#include "batchdrawer.h"

#define DEFAULT_ZOOM 12.0f

// TODO: rename this file (what is common...?) and refactor to make it less monolothic/disorganized/awful

using glm::vec3;
using glm::mat4;
using glm::quat;

static bool quit = false;

// TODO: replace with scene class
struct {
    SDL_Window* window;

    glm::mat4 camera_matrix, model_matrix;
    glm::vec3 camera_position;

    Arcball arcball{};
    Camera camera{};

    bool isArcball{true};

    Tree* tree;
    std::unique_ptr<batchdrawer> treeDrawer;
    unsigned int ticks{0};
    float zoom{1.0f};
} scene;

//TODO: replace with material
struct {
    GLuint program{0};
    GLint uniform_camera{0}, uniform_model{0}, uniform_color{0}, uniform_camera_position{0};
    GLint attr_position{0}, attr_normal{0};
} branchShader;

// TODO: Fix winding weirdness exhibited by this test branch
void initTestTree() {
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
    scene.tree = new Tree{root, *scene.treeDrawer};
}

void initDefaultTree() {
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
    scene.tree = new Tree{root, *scene.treeDrawer};
}

bool initResources() {
    // Shaders
    // TODO: do setup in shader/material object
    branchShader.program = create_program("../shaders/phong.v.glsl", "../shaders/phong.f.glsl");

    branchShader.attr_position = glGetAttribLocation(branchShader.program, "position");
    branchShader.attr_normal = glGetAttribLocation(branchShader.program, "normal");

    branchShader.uniform_camera = glGetUniformLocation(branchShader.program, "camera");
    branchShader.uniform_model = glGetUniformLocation(branchShader.program, "model");
    branchShader.uniform_color = glGetUniformLocation(branchShader.program, "color");
    branchShader.uniform_camera_position = glGetUniformLocation(branchShader.program, "cameraPosition");

    scene.treeDrawer = std::make_unique<batchdrawer>();
    initDefaultTree();

    return true;
}

void resizeWindow(int w, int h) {
    // Update cameras
    scene.arcball.setBounds((float) w, (float) h);
    scene.camera.setAspectRatio((float) w / h);

    glViewport(0, 0, w, h);
}

bool cInit(int w, int h, SDL_Window* window) {
    resizeWindow(w, h);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    scene.window = window;

    // Our geometry generation winds clockwise, while the default is anti-clockwise
    glFrontFace(GL_CW);

    return true;
}

void updateMVP() {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    scene.model_matrix = model;

    if (scene.isArcball) {
        scene.camera_position = (glm::vec3(0.0, 0.0, DEFAULT_ZOOM) * scene.zoom) * scene.arcball.getRotation();
        glm::mat4 view = glm::lookAt(scene.camera_position * scene.zoom, glm::vec3(0.0, 0.0, 0.0),
                                     WORLD_UP);
        scene.camera_matrix =
                glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 1000.0f) * view * toMat4(scene.arcball.getRotation());
    } else {
        scene.camera_position = scene.camera.position();
        scene.camera_matrix = scene.camera.matrix();
    }
}

void setCamera(bool isArcball) {
    if (isArcball) {
        SDL_SetRelativeMouseMode(SDL_FALSE);
    } else {
        SDL_SetRelativeMouseMode(SDL_TRUE);
    }
}

void updateTicks() {
    static float frac{0};
    static unsigned int lastTicks{0};

    unsigned int diff = SDL_GetTicks() - lastTicks;
    lastTicks += diff;

    frac += diff * TICKS_PER_MS;
    diff = (unsigned int) frac;

    frac -= diff;
    scene.ticks += diff;
    scene.tree->advance(diff);
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //TODO: do only when necessary
    updateMVP();

    // Draw trees
    glUseProgram(branchShader.program);
    glEnableVertexAttribArray(branchShader.attr_position);
    glEnableVertexAttribArray(branchShader.attr_normal);

    glUniformMatrix4fv(branchShader.uniform_camera, 1, GL_FALSE, (GLfloat*) &scene.camera_matrix);
    glUniformMatrix4fv(branchShader.uniform_model, 1, GL_FALSE, (GLfloat*) &scene.model_matrix);
    //glUniform3fv(branchShader.uniform_camera_position, 1, (GLfloat*) &scene.camera_position);
    glUniform3f(branchShader.uniform_camera_position, 0.0f, 0.0f, 1.0f);
    glUniform4f(branchShader.uniform_color, 0.4f, 0.9f, 0.45f, 1.0f);

    scene.treeDrawer->prepareToDraw();
    glVertexAttribPointer(branchShader.attr_position, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
                          (void*) offsetof(vertex, position));
    glVertexAttribPointer(branchShader.attr_normal, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
                          (void*) offsetof(vertex, normal));
    scene.treeDrawer->draw();

    glDisableVertexAttribArray(branchShader.attr_position);
    glDisableVertexAttribArray(branchShader.attr_normal);

    SDL_GL_SwapWindow(scene.window);
}

struct KeysPressed {
    bool w{false}, a{false}, s{false}, d{false};
};

void handleKeyboard(const KeysPressed& keys) {
    constexpr float moveSpeed = 0.01f;
    static unsigned int lastTicks{0};
    unsigned int ticks = SDL_GetTicks();
    unsigned int ms = ticks - lastTicks;
    lastTicks = ticks;

    if (keys.w) scene.camera.offsetPosition(ms * moveSpeed * scene.camera.front());
    if (keys.s) scene.camera.offsetPosition(ms * moveSpeed * -scene.camera.front());
    if (keys.a) scene.camera.offsetPosition(ms * moveSpeed * scene.camera.right());
    if (keys.d) scene.camera.offsetPosition(ms * moveSpeed * -scene.camera.right());
}

void handleMouseDrag(bool mouseDown, SDL_Event e) {
    constexpr float turnSensitivity = 0.05f;
    if (scene.isArcball && mouseDown) {
        scene.arcball.update(e.motion.x, e.motion.y);
    } else if (!scene.isArcball) {
        scene.camera.offsetOrientation(turnSensitivity * e.motion.xrel, turnSensitivity * e.motion.yrel);
    }
}

void handleMouseWheel(float dy) {
    if (dy > 0) { // scroll up
        scene.zoom *= 0.8;
    } else if (dy < 0) { // scroll down
        scene.zoom *= 1.2;
    }
}

void update() {
    SDL_Event e;
    static KeysPressed keys{};
    static bool mouseDown;

    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            quit = true;
        } else if (e.type == SDL_MOUSEBUTTONDOWN) {
            mouseDown = true;
            if (scene.isArcball) {
                scene.arcball.startDrag(e.button.x, e.button.y);
            }
        } else if (e.type == SDL_MOUSEBUTTONUP) {
            mouseDown = false;
        } else if (e.type == SDL_MOUSEMOTION) {
            handleMouseDrag(mouseDown, e);
        } else if (e.type == SDL_MOUSEWHEEL) {
            handleMouseWheel(e.wheel.y);
        } else if (e.type == SDL_WINDOWEVENT) {
            if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
                resizeWindow(e.window.data1, e.window.data2);
            }
        } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_r:
                    scene.isArcball = !scene.isArcball;
                    setCamera(scene.isArcball);
                    break;
                case SDLK_ESCAPE:
                    scene.isArcball = true;
                    setCamera(scene.isArcball);
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

    handleKeyboard(keys);
    updateTicks();
    render();
}

void cClose() {
    glDeleteProgram(branchShader.program);
    SDL_Quit();
}

bool shouldQuit() {
    return quit;
}

Tree* getSceneTree() {
    return scene.tree;
}
