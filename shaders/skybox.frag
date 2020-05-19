#version 330 core

in vec3 ourNormal;
in vec3 FragPosition;
in vec2 ourTexCoords;
out vec4 FragColor;

uniform sampler2D tex;

void main() {
    vec3 normal   = normalize(ourNormal);
    vec3 color = texture(tex, ourTexCoords).rgb;
    FragColor = vec4(color, 1.0);
}
