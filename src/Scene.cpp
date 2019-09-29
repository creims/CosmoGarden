#include <SDL_events.h>
#include <glm/gtc/matrix_transform.hpp>
#include "Scene.h"

Scene::Scene(SDL_Window* wnd)
        : window{wnd} {}

void Scene::toggleCamera() {
    isArcball = !isArcball;
    updateCameras();
}

void Scene::defaultCamera() {
    isArcball = true;
}

void Scene::handleMouseDown(int x, int y) {
    if(isArcball) {
        arcball.startDrag(x, y);
    }
}

void Scene::handleMouseDrag(int x, int y, bool mouseDown) {
    constexpr float turnSensitivity = 0.05f;
    if (isArcball && mouseDown) {
        arcball.update(x, y);
    } else if (!isArcball) {
        camera.offsetOrientation(turnSensitivity * x, turnSensitivity * y);
    }
}

bool Scene::isArcballCamera() const {
    return isArcball;
}

void Scene::cameraForward(float magnitude) {
    camera.offsetPosition(magnitude * camera.front());
}

void Scene::cameraBackward(float magnitude) {
    camera.offsetPosition(magnitude * -camera.front());
}

void Scene::cameraLeft(float magnitude) {
    camera.offsetPosition(magnitude * camera.right());
}

void Scene::cameraRight(float magnitude) {
    camera.offsetPosition(magnitude * -camera.right());
}

void Scene::cameraZoom(float ratio) {
    zoom *= ratio;
}

void Scene::updateCameras() {
    constexpr float default_zoom = 12.0f;
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    model_matrix = model;

    if (isArcball) {
        camera_position = (glm::vec3(0.0, 0.0, default_zoom) * zoom) * arcball.getRotation();
        glm::mat4 view = glm::lookAt(camera_position * zoom, glm::vec3(0.0, 0.0, 0.0),
                                     WORLD_UP);
        camera_matrix =
                glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 1000.0f) * view * toMat4(arcball.getRotation());
    } else {
        camera_position = camera.position();
        camera_matrix = camera.matrix();
    }
}

void Scene::render() {
    glClear((unsigned)GL_COLOR_BUFFER_BIT | (unsigned)GL_DEPTH_BUFFER_BIT);

    //TODO: do only when necessary
    updateCameras();

    material.use();

    // Set uniforms
    glUniformMatrix4fv(material.uniform_camera, 1, GL_FALSE, (GLfloat*) &camera_matrix);
    glUniformMatrix4fv(material.uniform_model, 1, GL_FALSE, (GLfloat*) &model_matrix);
    glUniform3fv(material.uniform_camera_position, 1, (GLfloat*) &camera_position);
    glUniform4f(material.uniform_color, 0.4f, 0.9f, 0.45f, 1.0f);

    // Draw plants
    treeDrawer.prepareToDraw();
    glVertexAttribPointer(material.attr_position, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
                          (void*) offsetof(vertex, position));
    glVertexAttribPointer(material.attr_normal, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
                          (void*) offsetof(vertex, normal));
    treeDrawer.draw();

    material.stopUsing();

    SDL_GL_SwapWindow(window);
}

void Scene::resizeWindow(int width, int height) {
    // Update cameras
    arcball.setBounds((float) width, (float) height);
    camera.setAspectRatio((float) width / height);

    glViewport(0, 0, width, height);
}

void Scene::buildTree(plantRoot const& root) {
    tree = std::make_unique<Tree>(root, treeDrawer);
}

void Scene::advanceTime(unsigned int ticks) {
    tree->advance(ticks);
}
