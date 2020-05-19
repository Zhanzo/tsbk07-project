#version 330 core

in vec3 ourNormal;

out vec4 FragColor;

uniform vec3 editable_color;

void main() {
    const vec3 light = normalize(vec3(0.58, 0.58, 0.58));
    vec3 normal      = normalize(ourNormal);
    float shade      = clamp(dot(light, normal), 0, 1);

    FragColor = vec4(vec3(shade) * editable_color, 1.0);
}