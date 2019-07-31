#include <iostream>

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

using glm::vec3;
using glm::mat4;
using glm::quat;

static bool quit = false;

// TODO: replace with scene class
struct {
    int width, height;
    SDL_Window* window;

    glm::mat4 p_matrix, mv_matrix;

    Arcball arcball{};
    quat camRotation{QUAT_IDENTITY};

    bool dragging{false};
    Tree* tree;
    batchdrawer* treeDrawer;
    unsigned int ticks{0};
} scene;

//TODO: replace with shader class?
struct {
    GLuint program{0};
    GLint uniform_p{0}, uniform_mv{0}, uniform_color{0};
    GLint attr_position{0}, attr_normal{0};
} branchShader;

bool initResources() {
    // Shaders
    // TODO: do setup in shader object
    branchShader.program = create_program("../shaders/phong.v.glsl", "../shaders/phong.f.glsl");

    branchShader.attr_position = glGetAttribLocation(branchShader.program, "position");
    branchShader.attr_normal = glGetAttribLocation(branchShader.program, "normal");

    branchShader.uniform_p = glGetUniformLocation(branchShader.program, "p_matrix");
    branchShader.uniform_mv = glGetUniformLocation(branchShader.program, "mv_matrix");
    branchShader.uniform_color = glGetUniformLocation(branchShader.program, "color");

    scene.treeDrawer = new batchdrawer();
    scene.tree = new Tree{
        Bezier{
            vec3{-0.6f, -2.0f, 0.0f},
            vec3{-0.55f, -1.3f, 0.0f},
            vec3{-0.4f, 0.7f, 0.0f},
            vec3{-0.5f, 1.4f, 0.0f},
    }, 100, *scene.treeDrawer};

    return true;
}

bool cInit(int w, int h, SDL_Window* window) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    scene.width = w;
    scene.height = h;
    scene.window = window;

    glFrontFace(GL_CW);

    // Initialize arcball
    scene.arcball.setBounds((float) scene.width, (float) scene.height);

    return true;
}

void updateMVP() {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, -4.0));
    glm::mat4 view = glm::lookAt(glm::vec3(1.0, 1.0, 1.0), glm::vec3(0.0, 0.0, -4.0), glm::vec3(0.0, 1.0, 0.0));

    scene.p_matrix = glm::perspective(45.0f, 1.0f * scene.width / scene.height, 0.1f, 10.0f);
    scene.mv_matrix = view * model * toMat4(scene.camRotation);
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

    glUniformMatrix4fv(branchShader.uniform_p, 1, GL_FALSE, (GLfloat*)&scene.p_matrix);
    glUniformMatrix4fv(branchShader.uniform_mv, 1, GL_FALSE, (GLfloat*)&scene.mv_matrix);
    glUniform4f(branchShader.uniform_color, 0.4f, 0.9f, 0.45f, 1.0f);

    scene.treeDrawer->prepareToDraw();
    glVertexAttribPointer(branchShader.attr_position, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, position));
    glVertexAttribPointer(branchShader.attr_normal, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, normal));
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
            scene.arcball.startDrag(e.button.x, e.button.y, scene.camRotation);
        } else if (e.type == SDL_MOUSEBUTTONUP) {
            scene.dragging = false;
        } else if (e.type == SDL_MOUSEMOTION) {
            if (scene.dragging) {
                scene.camRotation = scene.arcball.update(e.motion.x, e.motion.y);
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
