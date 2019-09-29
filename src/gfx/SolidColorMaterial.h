#ifndef SOLIDCOLORMATERIAL_H
#define SOLIDCOLORMATERIAL_H

#include <GL/glew.h>
#include "Material.h"

class SolidColorMaterial : public Material {
public:
    SolidColorMaterial();
    ~SolidColorMaterial() override;

    void use() override;

    void stopUsing() override;

    // TODO: make these private and use setter methods
    GLuint program{0};
    GLint uniform_camera{0}, uniform_model{0}, uniform_color{0}, uniform_camera_position{0};
    GLint attr_position{0}, attr_normal{0};
};

#endif //SOLIDCOLORMATERIAL_H
