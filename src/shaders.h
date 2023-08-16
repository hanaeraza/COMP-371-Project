#ifndef PROCEDURALWORLD_SHADERS_H
#define PROCEDURALWORLD_SHADERS_H

#define GLEW_STATIC 1   // This allows linking with Static Library on Windows, without DLL
#include <GL/glew.h>    // Include GLEW - OpenGL Extension Wrangler
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

using namespace glm;
using namespace std;

const char *SCENE_VERT = "#version 330 core\n"
                         "\n"
                         "uniform vec3 view_position;\n"
                         "\n"
                         "layout (location = 0) in vec3 position;\n"
                         "layout (location = 1) in vec3 normals;\n"
                         "layout (location = 2) in vec2 uv;\n"
                         "\n"
                         "uniform mat4 model_matrix;\n"
                         "uniform mat4 view_matrix;\n"
                         "uniform mat4 projection_matrix;\n"
                         "uniform mat4 light_view_proj_matrix;\n"
                         "\n"
                         "out vec3 fragment_normal;\n"
                         "out vec3 fragment_position;\n"
                         "out vec4 fragment_position_light_space;\n"
                         "out vec2 vertexUV;\n"
                         "\n"
                         "void main()\n"
                         "{\n"
                         "    vertexUV = uv;\n"
                         "    fragment_normal = mat3(model_matrix) * normals;\n"
                         "    fragment_position = vec3(model_matrix * vec4(position, 1.0));\n"
                         "    fragment_position_light_space = light_view_proj_matrix * vec4(fragment_position, 1.0);\n"
                         "    gl_Position = projection_matrix * view_matrix * model_matrix * vec4(position, 1.0);\n"
                         "}";

const char *SCENE_FRAG = "#version 330 core\n"
                         "\n"
                         "const float PI = 3.1415926535897932384626433832795;\n"
                         "\n"
                         "uniform vec3 light_color;\n"
                         "uniform vec3 light_position;\n"
                         "uniform vec3 light_direction;\n"
                         "\n"
                         "uniform vec3 light_color2;\n"
                         "uniform vec3 light_position2;\n"
                         "uniform vec3 light_direction2;\n"
                         "\n"
                         "uniform vec3 object_color;\n"
                         "uniform sampler2D textureSampler;\n"
                         "uniform bool useTexture = true;\n"
                         "uniform bool useCarLight;\n"
                         "uniform bool lightsOn;\n"
                         "\n"
                         "\n"
                         "const float shading_ambient_strength    = 0.1;\n"
                         "const float shading_diffuse_strength    = 1.0;\n"
                         "const float shading_specular_strength   = 1.0;\n"
                         "\n"
                         "const float shading_ambient_strength2 = 0.7;\n"
                         "const float shading_diffuse_strength2 = 0.6;\n"
                         "const float shading_specular_strength2 = 0.3;\n"
                         "\n"
                         "uniform float light_cutoff_outer;\n"
                         "uniform float light_cutoff_inner;\n"
                         "uniform float light_near_plane;\n"
                         "uniform float light_far_plane;\n"
                         "\n"
                         "uniform float light_cutoff_inner2;\n"
                         "uniform float light_cutoff_outer2;\n"
                         "uniform float light_near_plane2;\n"
                         "uniform float light_far_plane2;\n"
                         "\n"
                         "uniform float intensity;\n"
                         "\n"
                         "uniform vec3 view_position;\n"
                         "\n"
                         "uniform sampler2D shadow_map;\n"
                         "\n"
                         "in vec3 fragment_position;\n"
                         "in vec4 fragment_position_light_space;\n"
                         "in vec4 fragment_position_light_space2;\n"
                         "in vec3 fragment_normal;\n"
                         "in vec2 vertexUV;\n"
                         "\n"
                         "in vec4 gl_FragCoord;\n"
                         "\n"
                         "out vec4 result;\n"
                         "\n"
                         "vec3 ambient_color(vec3 light_color_arg) {\n"
                         "    return shading_ambient_strength * light_color_arg;\n"
                         "}\n"
                         "\n"
                         "vec3 diffuse_color(vec3 light_color_arg, vec3 light_position_arg, vec3 light_direction_arg) {\n"
                         "    return shading_diffuse_strength * light_color_arg * max(dot(normalize(fragment_normal), light_direction_arg), 0.0f);\n"
                         "}\n"
                         "\n"
                         "vec3 specular_color(vec3 light_color_arg, vec3 light_position_arg, vec3 light_direction_arg) {\n"
                         "    vec3 view_direction = normalize(view_position - fragment_position);\n"
                         "    vec3 reflect_light_direction = reflect(-light_direction_arg, normalize(fragment_normal));\n"
                         "    return shading_specular_strength * light_color_arg * pow(max(dot(reflect_light_direction, view_direction), 0.0f), 32);\n"
                         "}\n"
                         "\n"
                         "float shadow_scalar() {\n"
                         "    // this function returns 1.0 when the surface receives light, and 0.0 when it is in a shadow\n"
                         "    // perform perspective divide\n"
                         "    vec3 normalized_device_coordinates = fragment_position_light_space.xyz / fragment_position_light_space.w;\n"
                         "    // transform to [0,1] range\n"
                         "    normalized_device_coordinates = normalized_device_coordinates * 0.5 + 0.5;\n"
                         "    // get closest depth value from light's perspective (using [0,1] range fragment_position_light_space as coords)\n"
                         "    float closest_depth = texture(shadow_map, normalized_device_coordinates.xy).r;\n"
                         "    // get depth of current fragment from light's perspective\n"
                         "    float current_depth = normalized_device_coordinates.z;\n"
                         "    // check whether current frag pos is in shadow\n"
                         "    float bias = 0.003;// bias applied in depth map: see shadow_vertex.glsl\n"
                         "    float shadow = ((current_depth - bias) < closest_depth) ? 1.0 : 0.0;\n"
                         "\n"
                         "    if (normalized_device_coordinates.z > 1.0)\n"
                         "    shadow = 0.0;\n"
                         "    return shadow;\n"
                         "}\n"
                         "\n"
                         "float spotlight_scalar() {\n"
                         "    float theta = dot(normalize(fragment_position - light_position), light_direction);\n"
                         "\n"
                         "    if (theta > light_cutoff_inner) {\n"
                         "        return 1.0;\n"
                         "    } else if (theta > light_cutoff_outer) {\n"
                         "        return (1.0 - cos(PI * (theta - light_cutoff_outer) / (light_cutoff_inner - light_cutoff_outer))) / 2.0;\n"
                         "    } else {\n"
                         "        return 0.0;\n"
                         "    }\n"
                         "}\n"
                         "\n"
                         "void main()\n"
                         "{\n"
                         "    vec3 ambient = vec3(0.0f);\n"
                         "    vec3 diffuse = vec3(0.0f);\n"
                         "    vec3 specular = vec3(0.0f);\n"
                         "\n"
                         "    vec3 diffuse2 = vec3(0.0f);\n"
                         "    vec3 specular2 = vec3(0.0f);\n"
                         "\n"
                         "    vec3 light_dir = normalize(light_position - fragment_position);\n"
                         "\n"
                         "    float scalar = shadow_scalar() * spotlight_scalar();\n"
                         "    ambient = ambient_color(light_color);\n"
                         "    diffuse = scalar * diffuse_color(light_color, light_position, light_dir);\n"
                         "    specular = scalar * specular_color(light_color, light_position, light_dir);\n"
                         "    vec3 lightColor = vec3(0.0f);\n"
                         "\n"
                         "    vec3 objColor = useTexture ? texture(textureSampler, vertexUV).rgb : object_color;\n"
                         "\n"
                         "    vec3 color;\n"
                         "    if (useCarLight){\n"
                         "        diffuse2 =  spotlight_scalar()* shading_diffuse_strength2 *diffuse_color(light_color2, light_position2, light_direction);\n"
                         "        specular2 =  spotlight_scalar()*shading_specular_strength2 *specular_color(light_color2, light_position2, light_direction);\n"
                         "        lightColor = specular2 + diffuse2 + diffuse + specular;\n"
                         "    }\n"
                         "\n"
                         "    color = ((intensity * (ambient + ambient)) + lightColor) * objColor;\n"
                         "\n"
                         "    result = vec4(color, 1.0f);\n"
                         "}\n"
                         "";

const char *SKYBOX_VERT = "#version 330 core\n"
                          "\n"
                          "layout (location = 0) in vec3 position;\n"
                          "\n"
                          "out vec3 texCoords;\n"
                          "\n"
                          "uniform mat4 projection;\n"
                          "uniform mat4 view;\n"
                          "\n"
                          "void main()\n"
                          "{\n"
                          "    texCoords = position;\n"
                          "\n"
                          "    // The translation component is removed from the view to make the skybox position stationary\n"
                          "    vec4 pos = projection * mat4(mat3(view)) * vec4(position, 1.0);\n"
                          "\n"
                          "    // Give skybox the highest possible depth value to always make it appear behind all other objects\n"
                          "    gl_Position = pos.xyww;\n"
                          "}";

const char *SKYBOX_FRAG = "#version 330 core\n"
                          "\n"
                          "out vec4 fragColor;\n"
                          "\n"
                          "in vec3 texCoords;\n"
                          "\n"
                          "uniform samplerCube skybox;\n"
                          "uniform vec3 lightColor;\n"
                          "uniform float ambientStrength;\n"
                          "\n"
                          "void main()\n"
                          "{\n"
                          "    vec3 ambientColor = ambientStrength * lightColor * texture(skybox, texCoords).rgb;\n"
                          "\n"
                          "    fragColor = vec4(ambientColor, 1.0f);\n"
                          "}";

const char *SHADOW_VERT = "#version 330 core\n"
                          "layout (location = 0) in vec3 position;\n"
                          "\n"
                          "uniform mat4 light_view_proj_matrix;\n"
                          "uniform mat4 model_matrix;\n"
                          "\n"
                          "void main()\n"
                          "{\n"
                          "    mat4 scale_bias_matrix = mat4(vec4(0.5, 0.0, 0.0, 0.0),\n"
                          "                                    vec4(0.0, 0.5, 0.0, 0.0),\n"
                          "                                    vec4(0.0, 0.0, 0.5, 0.0),\n"
                          "                                    vec4(0.5, 0.5, 0.5, 1.0));\n"
                          "    gl_Position = \n"
                          "//                    scale_bias_matrix * // bias the depth map coordinates\n"
                          "                    light_view_proj_matrix * model_matrix * vec4(position, 1.0);\n"
                          "}";

const char *SHADOW_FRAG = "#version 330 core\n"
                          "\n"
                          "out vec4 FragColor;\n"
                          "\n"
                          "in vec4 gl_FragCoord;\n"
                          "\n"
                          "void main()\n"
                          "{\n"
                          "    gl_FragDepth = gl_FragCoord.z;\n"
                          "\n"
                          "    FragColor = vec4(vec3(gl_FragCoord.z), 1.0f);\n"
                          "}";

// Returns shader program ID
int compileAndLinkShaders(const char *vertexShaderSrc, const char *fragmentShaderSrc) {
    
    // Vertex shader
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSrc, nullptr);
    glCompileShader(vertexShader);
    
    // Check for vertex shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    // Fragment shader
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSrc, nullptr);
    glCompileShader(fragmentShader);
    
    // Check for fragment shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    // Link shaders
    int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    // Check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return shaderProgram;
}

// shader variable setters
void SetUniformMat4(GLuint shader_id, const char *uniform_name, mat4 uniform_value) {
    glUseProgram(shader_id);
    glUniformMatrix4fv(glGetUniformLocation(shader_id, uniform_name), 1, GL_FALSE, &uniform_value[0][0]);
}

void SetUniformVec3(GLuint shader_id, const char *uniform_name, vec3 uniform_value) {
    glUseProgram(shader_id);
    glUniform3fv(glGetUniformLocation(shader_id, uniform_name), 1, value_ptr(uniform_value));
}

template<class T>
void SetUniform1Value(GLuint shader_id, const char *uniform_name, T uniform_value) {
    glUseProgram(shader_id);
    glUniform1i(glGetUniformLocation(shader_id, uniform_name), uniform_value);
    glUseProgram(0);
}

template<class T>
void SetUniform1fValue(GLuint shader_id, const char *uniform_name, T uniform_value) {
    glUseProgram(shader_id);
    glUniform1f(glGetUniformLocation(shader_id, uniform_name), uniform_value);
    glUseProgram(0);
}

void setProjectionMatrix(int shaderProgram, mat4 projectionMatrix) {
    glUseProgram(shaderProgram);
    GLint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projection_matrix");
    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
}

void setViewMatrix(int shaderProgram, mat4 viewMatrix) {
    glUseProgram(shaderProgram);
    GLint viewMatrixLocation = glGetUniformLocation(shaderProgram, "view_matrix");
    glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
}

void setWorldMatrix(int shaderProgram, mat4 _worldMatrix) {
    glUseProgram(shaderProgram);
    GLint worldMatrixLocation = glGetUniformLocation(shaderProgram, "model_matrix");
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &_worldMatrix[0][0]);
}

#endif //PROCEDURALWORLD_SHADERS_H
