uniform vec4 color;
uniform mat4 camera, model;
uniform vec3 cameraPosition;

varying vec3 frag_position, frag_normal;

// The direction TOWARD our light source (avoids negating the dot product when calculating dot products)
const vec3 light_direction = normalize(vec3(0.408248, 0.816497, 0.608248));

// How much ambient light
const float ambientCoefficient = 0.2;

// Light colors
const vec3 light_diffuse = vec3(1.0, 1.0, 1.0);
const vec3 light_ambient = vec3(1.0, 1.0, 1.0);
const vec3 light_specular = vec3(1.0, 1.0, 1.0);

// Shininess is const for now instead of per fragment
const float shininess = 10.0;
const vec3 specular_color = vec3(1.0, 1.0, 1.0);

/*float cel(float d) {
    return smoothstep(0.35, 0.37, d) * 0.4 + smoothstep(0.70, 0.72, d) * 0.6;
}*/

float warp_diffuse(float d) {
    //return cel(d * 0.8 + 0.2);
    return d * 0.8 + 0.2;
}

void main() {
    vec3 normal = frag_normal;
    vec3 surfacePos = vec3(model * vec4(frag_position, 1.0));
    vec3 surfaceToCamera = normalize(cameraPosition - surfacePos);

    //Ambient
    vec3 ambient = ambientCoefficient * light_ambient * color.rgb;

    // Dotting the direction to the light with the normal gives the diffuse coefficient
    float diffuseCoefficient = max(warp_diffuse(dot(normal, light_direction)), 0.0);
    vec3 diffuse = diffuseCoefficient * color.rgb * light_diffuse;

    // Calculate specular component
    float specularCoefficient = 0.0;
    if(diffuseCoefficient > 0.0) {
        specularCoefficient = pow(max(0.0, dot(surfaceToCamera, reflect(-light_direction, normal))), shininess);
    }
    vec3 specular = specularCoefficient * specular_color * light_specular;

    // Finally, multiply by the color to give the final fragment color
    gl_FragColor = vec4(ambient + diffuse + specular, 1.0);
}
