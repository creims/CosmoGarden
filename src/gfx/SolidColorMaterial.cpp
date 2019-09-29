#include "SolidColorMaterial.h"
#include "shader_utils.h"

SolidColorMaterial::SolidColorMaterial() {
    program = create_program("../shaders/phong.v.glsl", "../shaders/phong.f.glsl");

    attr_position = glGetAttribLocation(program, "position");
    attr_normal = glGetAttribLocation(program, "normal");

    uniform_camera = glGetUniformLocation(program, "camera");
    uniform_model = glGetUniformLocation(program, "model");
    uniform_color = glGetUniformLocation(program, "color");
    uniform_camera_position = glGetUniformLocation(program, "cameraPosition");
}

void SolidColorMaterial::use() {
    glUseProgram(program);
    glEnableVertexAttribArray(attr_position);
    glEnableVertexAttribArray(attr_normal);
}

void SolidColorMaterial::stopUsing() {
    glDisableVertexAttribArray(attr_position);
    glDisableVertexAttribArray(attr_normal);
}

SolidColorMaterial::~SolidColorMaterial() {
    if(program != 0) glDeleteProgram(program);
}
