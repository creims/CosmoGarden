uniform vec4 color;
uniform mat4 p_matrix, mv_matrix;

varying vec3 frag_position, frag_normal;

// The direction TOWARD our light source (avoids negating the dot product when calculating the diffuse factor)
const vec3 light_direction = vec3(0.408248, 0.816497, 0.608248);

// How much of our brightest light is ambient versus diffuse
const vec4 light_diffuse = vec4(0.8, 0.8, 0.8, 0.0);
const vec4 light_ambient = vec4(0.2, 0.2, 0.2, 1.0);

void main() {
    // To make the light independent of the camera, use the next line instead. Using the untransformed light direction
    // makes the light move with the camera, making it as if you're rotating the model under a lamp
    //vec3 mv_light_direction = (mv_matrix * vec4(light_direction, 0.0)).xyz;
    vec3 mv_light_direction = light_direction;

    // The normal needs to be normalized to make it a direction vector
    vec3 normal = normalize(frag_normal);

    // Dotting the direction to the light with the normal gives the diffuse coefficient
    vec4 diffuse_factor = max(dot(normal, mv_light_direction), 0.0) * light_diffuse;

    // Add the ambient component so the fragment is never completely dark
    vec4 ambient_diffuse_factor = diffuse_factor + light_ambient;

    // Finally, multiply by the color to give the final fragment color
    gl_FragColor = ambient_diffuse_factor * color;
}
