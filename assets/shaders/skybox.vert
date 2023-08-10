#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;

    // The translation component is removed from the view to make the skybox position stationary
    vec4 pos = projection * mat4(mat3(view)) * vec4(aPos, 1.0);

    // Give skybox the highest possible depth value to always make it appear behind all other objects
    gl_Position = pos.xyww;
}