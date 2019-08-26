uniform mat4 camera, model;

attribute vec3 position, normal;

varying vec3 frag_position, frag_normal;

void main() {
    frag_position = position;
    frag_normal = normal;

    gl_Position = camera * model * vec4(position, 1.0);
}
