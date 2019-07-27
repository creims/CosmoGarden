uniform mat4 p_matrix, mv_matrix;

attribute vec3 position, normal;

varying vec3 frag_position, frag_normal;

void main() {
    vec4 eye_position = mv_matrix * vec4(position, 1.0);
    gl_Position = p_matrix * eye_position;

    frag_position = eye_position.xyz;
    frag_normal = (mv_matrix * vec4(normal, 0.0)).xyz;
}
