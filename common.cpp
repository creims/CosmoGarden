#include <iostream>

#include <math.h>
#include <SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
    GLint attr_coord3d{0}, uniform_mvp{0}, uniform_color{0};
} branchShader;

void initTree() {
    Bezier td = Bezier{
            vec3{1.0f, 2.0f, -0.5f},
            vec3{2.0f, -1.0f, 1.0f},
            vec3{0.0f, -0.5f, -2.0f},
            vec3{-1.0f, 2.0f, 1.5f},
    };

    Bezier tz = Bezier{
            vec3{0.0f, 0.0f, -2.0f},
            vec3{0.0f, 0.0f, -1.0f},
            vec3{0.0f, 0.0f, 1.0f},
            vec3{0.0f, 0.0f, 2.0f},
    };

    Bezier tx = Bezier{
            vec3{-2.0f, 0.0f, 0.0f},
            vec3{-1.0f, 0.0f, 0.0f},
            vec3{1.0f, 0.0f, 0.0f},
            vec3{2.0f, 0.0f, 0.0f},
    };

    Bezier ty = Bezier{
            vec3{0.0f, -2.0f, 0.0f},
            vec3{0.0f, -1.0f, 0.0f},
            vec3{0.0f, 1.0f, 0.0f},
            vec3{0.0f, 2.0f, 0.0},
    };

    Bezier ashTrunk{
            vec3{-0.6f, -2.0f, 0.0f},
            vec3{-0.55f, -1.3f, 0.0f},
            vec3{-0.4f, 0.7f, 0.0f},
            vec3{-0.5f, 1.4f, 0.0f},
    };

    Bezier trunk = ashTrunk;

    scene.tree = new Tree{trunk, 100, *scene.treeDrawer};
}

bool initResources() {
    // Shaders
    // TODO: do setup in shader object
    branchShader.program = create_program("../shaders/monocolor.v.glsl", "../shaders/monocolor.f.glsl");
    branchShader.attr_coord3d = glGetAttribLocation(branchShader.program, "coord3d");
    branchShader.uniform_mvp = glGetUniformLocation(branchShader.program, "m_transform");
    branchShader.uniform_color = glGetUniformLocation(branchShader.program, "color");

    scene.treeDrawer = new batchdrawer();
    initTree();

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

mat4 genMVP() {
    static float angle;
    vec3 axis_y(0, 1, 0);
    mat4 anim = rotate(mat4(1.0f), glm::radians(angle), axis_y);

    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, -4.0));
    glm::mat4 view = glm::lookAt(glm::vec3(1.0, 1.0, 1.0), glm::vec3(0.0, 0.0, -4.0), glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 projection = glm::perspective(45.0f, 1.0f * scene.width / scene.height, 0.1f, 10.0f);

    mat4 mvp = projection * view * model * toMat4(scene.camRotation);

    return mvp;
}

void updateTicks() {
    static float frac{0};
    static unsigned int lastTicks{0};

    unsigned int diff = SDL_GetTicks() - lastTicks;
    lastTicks += diff;

    if (true) {
        frac += diff * TICKS_PER_MS;
        diff = (unsigned int) frac;

        frac -= diff;
        scene.ticks += diff;
        scene.tree->advance(diff);
    }
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const float* mvp_ptr = value_ptr(genMVP());

    // Draw trees
    glUseProgram(branchShader.program);
    glEnableVertexAttribArray(branchShader.attr_coord3d);
    glUniformMatrix4fv(branchShader.uniform_mvp, 1, GL_FALSE, mvp_ptr);

    glUniform4f(branchShader.uniform_color, 1.0f, 0.0f, 0.0f, 1.0f);
    scene.treeDrawer->prepareToDraw();
    glVertexAttribPointer(branchShader.attr_coord3d, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    scene.treeDrawer->draw();

    glDisableVertexAttribArray(branchShader.attr_coord3d);

    // Draw curves
    /*
    glUseProgram(curveShader.program);
    glEnableVertexAttribArray(curveShader.attr_coord3d);
    glUniformMatrix4fv(curveShader.uniform_mvp, 1, GL_FALSE, mvp_ptr);

    for (auto &curve : scene.tree.drawableBeziers()) {
        curve->prepareToDraw();
        glVertexAttribPointer(curveShader.attr_coord3d, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        curve->draw();
    }

    glDisableVertexAttribArray(curveShader.attr_coord3d);
     */

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
