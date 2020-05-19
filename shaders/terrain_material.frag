#version 330 core

struct DirectionalLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct Material {
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

in vec3 ourNormal;
in vec3 FragPosition;
in vec2 ourTexCoords;
out vec4 FragColor;

uniform Material material;
uniform DirectionalLight dir_light;
uniform PointLight point_light[8];
uniform vec3 view_position;

vec3 dir_light_calculate(DirectionalLight light, vec3 color, vec3 normal,
                         vec3 view_dir) {
    vec3 light_dir = normalize(-light.direction);
    float diff     = max(dot(normal, light_dir), 0.0);

    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);

    vec3 ambient  = light.ambient * material.diffuse * color;
    vec3 diffuse  = light.diffuse * diff * material.diffuse * color;
    vec3 specular = light.specular * spec * material.specular;
    return (ambient + diffuse + specular);
}

vec3 point_light_calculate(PointLight light, vec3 color, vec3 normal,
                           vec3 view_dir, vec3 frag_position) {
    vec3 light_dir = -normalize(frag_position - light.position);
    float diff     = max(dot(normal, light_dir), 0.0);

    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);

    float distance    = length(light.position - frag_position);
    float denominator = (light.constant + light.linear * distance +
                         light.quadratic * (distance * distance));
    float attenuation = denominator == 0.0 ? 0.0 : (1.0 / denominator);

    vec3 ambient  = light.ambient * material.diffuse * color;
    vec3 diffuse  = light.diffuse * diff * material.diffuse * color;
    vec3 specular = light.specular * spec * material.specular;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

uniform sampler2D textures[4];
uniform sampler2D splatmap;

void main() {
    vec3 normal   = normalize(ourNormal);
    vec3 view_dir = normalize(view_position - FragPosition);

    vec2 splatmap_texcoords = ourTexCoords;
    vec4 splatmap_color     = texture(splatmap, splatmap_texcoords);

    vec2 texcoord = splatmap_texcoords * 10;

    vec3 color = vec3(0);
    color += texture(textures[0], texcoord).rgb * splatmap_color.r;
    color += texture(textures[1], texcoord).rgb * splatmap_color.b;
    color += texture(textures[2], texcoord).rgb * splatmap_color.g;
    color += texture(textures[3], texcoord).rgb *
             (1 - splatmap_color.r - splatmap_color.g - splatmap_color.b);

    vec3 result = dir_light_calculate(dir_light, color, normal, view_dir);
    for (int i = 0; i < 8; i++) {
        result += point_light_calculate(point_light[i], color, normal, view_dir,
                                        FragPosition);
    }

    FragColor = vec4(result, 1.0);
}
