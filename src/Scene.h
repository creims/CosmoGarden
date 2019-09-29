#ifndef SCENE_H
#define SCENE_H

#include <SDL_video.h>
#include "tree.h"
#include "Arcball.h"
#include "Camera.h"
#include "gfx/SolidColorMaterial.h"

class Scene {
public:
    explicit Scene(SDL_Window* window);

    void resizeWindow(int width, int height);

    void render();

    void toggleCamera();
    void defaultCamera();

    void handleMouseDown(int x, int y);
    void handleMouseDrag(int x, int y, bool mouseDown);

    void cameraZoom(float ratio);

    void cameraForward(float magnitude);
    void cameraBackward(float magnitude);
    void cameraLeft(float magnitude);
    void cameraRight(float magnitude);
    void advanceTime(unsigned int ticks);

    bool isArcballCamera() const;

    void buildTree(plantRoot const& root);


private:
    SDL_Window* window;
    batchdrawer treeDrawer;
    glm::mat4 camera_matrix{}, model_matrix{};
    glm::vec3 camera_position{};
    Arcball arcball{};
    Camera camera{};

    // TODO: model instances instead of one model, eg vector<unique_ptr<GameObject>> orsmth
    std::unique_ptr<Tree> tree{nullptr};

    bool isArcball{true};
    float zoom{1.0f};

    void updateCameras();

    SolidColorMaterial material;
};


#endif //SCENE_H
