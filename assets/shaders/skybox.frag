#version 330 core

out vec4 fragColor;

in vec3 texCoords;

uniform samplerCube skybox;
uniform vec3 lightColor;

void main()
{
    float ambientStrength = 0.8f;
    vec3 ambientColor = ambientStrength * lightColor * texture(skybox, texCoords).rgb;

    fragColor = vec4(ambientColor, 1.0f);
}