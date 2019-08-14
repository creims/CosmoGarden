#include <iostream>

#include <memory>
#include <math.h>
#include <SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stddef.h>

#include "common.h"
#include "shader_utils.h"
#include "tree.h"
#include "consts.h"
#include "arcball.h"
#include "batchdrawer.h"

#define DEFAULT_ZOOM 12.0f

using glm::vec3;
using glm::mat4;
using glm::quat;

static bool quit = false;

// TODO: replace with scene class
struct {
    SDL_Window* window;

    glm::mat4 p_matrix, mv_matrix;

    Arcball arcball{};
    //quat camRotation{QUAT_IDENTITY};

    bool dragging{false};
    Tree* tree;
    std::unique_ptr<batchdrawer> treeDrawer;
    unsigned int ticks{0};
    float zoom{1.0f};
} scene;

//TODO: replace with shader class?
struct {
    GLuint program{0};
    GLint uniform_p{0}, uniform_mv{0}, uniform_color{0};
    GLint attr_position{0}, attr_normal{0};
} branchShader;

void initTestTree() {
    branchDescription trunk{};
    branchDescription branch1{};
    branch1.startRatio = 0.6f;
    branchDescription branch2 = branch1;
    branch2.angle = 120.0f;
    branchDescription branch3 = branch1;
    branch3.angle = 240.0f;
    trunk.children.push_back(branch1);
    //trunk.children.push_back(branch2);
    //trunk.children.push_back(branch3);

    scene.tree = new Tree{trunk, *scene.treeDrawer};
}

void initDefaultTree() {
    branchDescription trunk{};
    trunk.curve = refBranch{
            0.5f, -3.0f, -0.1f,
            0.2f, -1.0f, 0.1f,
            -0.5f, 2.0f, 0.3f,
            0.0f, 3.5f, 0.1f
    };

    crectionScaleFunc def = [](float growthPct, float distAlongCurve) {
        return 0.4 * growthPct * powf(1.0f - distAlongCurve, 0.3f);
    };

    trunk.getCrectionScale = def;

    branchDescription branch1{};
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

    scene.tree = new Tree{trunk, *scene.treeDrawer};
    scene.tree->buildFromTrunk(trunk);
}

bool initResources() {
    // Shaders
    // TODO: do setup in shader object
    branchShader.program = create_program("../shaders/phong.v.glsl", "../shaders/phong.f.glsl");

    branchShader.attr_position = glGetAttribLocation(branchShader.program, "position");
    branchShader.attr_normal = glGetAttribLocation(branchShader.program, "normal");

    branchShader.uniform_p = glGetUniformLocation(branchShader.program, "p_matrix");
    branchShader.uniform_mv = glGetUniformLocation(branchShader.program, "mv_matrix");
    branchShader.uniform_color = glGetUniformLocation(branchShader.program, "color");

    scene.treeDrawer = std::make_unique<batchdrawer>();
    initDefaultTree();

    return true;
}

void resizeWindow(int w, int h) {
    scene.p_matrix = glm::perspective(glm::radians(45.0f), 1.0f * w / h, 0.1f, 1000.0f);
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

    // Initialize arcball
    scene.arcball.setBounds((float) w, (float) h);

    return true;
}

void updateMV() {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0));
    glm::mat4 view = glm::lookAt(glm::vec3(0.0, 0.0, DEFAULT_ZOOM) * scene.zoom, glm::vec3(0.0, 0.0, 0.0), WORLD_UP);

    scene.mv_matrix = view * model * toMat4(scene.arcball.getRotation());
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
    updateMV();

    // Draw trees
    glUseProgram(branchShader.program);
    glEnableVertexAttribArray(branchShader.attr_position);
    glEnableVertexAttribArray(branchShader.attr_normal);

    glUniformMatrix4fv(branchShader.uniform_p, 1, GL_FALSE, (GLfloat*) &scene.p_matrix);
    glUniformMatrix4fv(branchShader.uniform_mv, 1, GL_FALSE, (GLfloat*) &scene.mv_matrix);
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

void update() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            quit = true;
        } else if (e.type == SDL_MOUSEBUTTONDOWN) {
            scene.dragging = true;
            scene.arcball.startDrag(e.button.x, e.button.y);
        } else if (e.type == SDL_MOUSEBUTTONUP) {
            scene.dragging = false;
        } else if (e.type == SDL_MOUSEMOTION) {
            if (scene.dragging) {
                scene.arcball.update(e.motion.x, e.motion.y);
            }
        } else if (e.type == SDL_MOUSEWHEEL) {
            if (e.wheel.y > 0) { // scroll up
                scene.zoom *= 0.8;
            } else if (e.wheel.y < 0) { // scroll down
                scene.zoom *= 1.2;
            }
        } else if (e.type == SDL_WINDOWEVENT) {
            if(e.window.event == SDL_WINDOWEVENT_RESIZED) {
                resizeWindow(e.window.data1, e.window.data2);
            }
        }
    }

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
