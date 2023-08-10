// Assignment 2

#include <iostream>
#include <list>
#include <algorithm>
#include <time.h>
#include <vector>


#define GLEW_STATIC 1   // This allows linking with Static Library on Windows, without DLL

#include <GL/glew.h>    // Include GLEW - OpenGL Extension Wrangler

#include <GLFW/glfw3.h> // GLFW provides a cross-platform interface for creating a graphical context,
// initializing OpenGL and binding inputs

#include <glm/glm.hpp>  // GLM is an optimized math library with syntax to similar to OpenGL Shading Language
#include <glm/gtc/matrix_transform.hpp> // include this to create transformation matrices
#include <glm/common.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaderloader.h"

#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>

using namespace glm;
using namespace std;

unsigned int indexCount;

int createTexturedCubeVAO();

int createSphereObject();

GLuint loadTexture(const char *filename);

void renderScene(GLuint shader, int texturedCubeVAO, int sphereVAO, GLuint tennisTextureID, GLuint glossyTextureID,
                 GLuint clayTextureID, GLuint noTextureID);

// Translation keyboard input variables
vec3 position(0.0f);
float rotation = 0.0f;
vec3 rotationAngle = vec3(0.0f, 1.0f, 0.0f);
vec3 scaling(1.0f);
float fov = 70.0f;
// Hierarchical Modeling

mat4 partMatrix = scale(mat4(1.0f), vec3(-1.0f, 1.0f, 1.0f));
mat4 groupMatrix = translate(mat4(1.0f), vec3(0.0f + position.x, 0.0f + position.y, 0.0f + position.z)) *
                   scale(mat4(1.0f), vec3(0.1f + scaling.x, 0.1f + scaling.y, 0.1f + scaling.z)) *
                   rotate(mat4(1.0f), radians(0.0f + rotation), vec3(0.0f, 1.0f, 0.0f));
mat4 worldMatrix = groupMatrix * partMatrix;

// window dimensions
const GLuint WIDTH = 1024, HEIGHT = 768;

GLuint setupModelVBO(string path, int &vertexCount);

//Sets up a model using an Element Buffer Object to refer to vertex data
GLuint setupModelEBO(string path, int &vertexCount);


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

GLFWwindow *window = nullptr;

bool InitContext();


struct TexturedColoredVertex {
    TexturedColoredVertex(vec3 _position, vec3 _normal, vec2 _uv)
            : position(_position), normal(_normal), uv(_uv) {}
    
    vec3 position;
    vec3 normal;
    vec2 uv;
};


// Cube model
const TexturedColoredVertex texturedCubeVertexArray[] = {  // position, normal, uv
        TexturedColoredVertex(vec3(-0.5f, -0.5f, -0.5f), vec3(-1.0f, 0.0f, 0.0f), vec2(0.0f, 0.0f)), //left
        TexturedColoredVertex(vec3(-0.5f, -0.5f, 0.5f), vec3(-1.0f, 0.0f, 0.0f), vec2(0.0f, 1.0f)),
        TexturedColoredVertex(vec3(-0.5f, 0.5f, 0.5f), vec3(-1.0f, 0.0f, 0.0f), vec2(1.0f, 1.0f)),
        
        TexturedColoredVertex(vec3(-0.5f, -0.5f, -0.5f), vec3(-1.0f, 0.0f, 0.0f), vec2(0.0f, 0.0f)),
        TexturedColoredVertex(vec3(-0.5f, 0.5f, 0.5f), vec3(-1.0f, 0.0f, 0.0f), vec2(1.0f, 1.0f)),
        TexturedColoredVertex(vec3(-0.5f, 0.5f, -0.5f), vec3(-1.0f, 0.0f, 0.0f), vec2(1.0f, 0.0f)),
        
        TexturedColoredVertex(vec3(0.5f, 0.5f, -0.5f), vec3(0.0f, 0.0f, -1.0f), vec2(1.0f, 1.0f)), // far
        TexturedColoredVertex(vec3(-0.5f, -0.5f, -0.5f), vec3(0.0f, 0.0f, -1.0f), vec2(0.0f, 0.0f)),
        TexturedColoredVertex(vec3(-0.5f, 0.5f, -0.5f), vec3(0.0f, 0.0f, -1.0f), vec2(0.0f, 1.0f)),
        
        TexturedColoredVertex(vec3(0.5f, 0.5f, -0.5f), vec3(0.0f, 0.0f, -1.0f), vec2(1.0f, 1.0f)),
        TexturedColoredVertex(vec3(0.5f, -0.5f, -0.5f), vec3(0.0f, 0.0f, -1.0f), vec2(1.0f, 0.0f)),
        TexturedColoredVertex(vec3(-0.5f, -0.5f, -0.5f), vec3(0.0f, 0.0f, -1.0f), vec2(0.0f, 0.0f)),
        
        TexturedColoredVertex(vec3(0.5f, -0.5f, 0.5f), vec3(0.0f, -1.0f, 0.0f), vec2(1.0f, 1.0f)), // bottom
        TexturedColoredVertex(vec3(-0.5f, -0.5f, -0.5f), vec3(0.0f, -1.0f, 0.0f), vec2(0.0f, 0.0f)),
        TexturedColoredVertex(vec3(0.5f, -0.5f, -0.5f), vec3(0.0f, -1.0f, 0.0f), vec2(1.0f, 0.0f)),
        
        TexturedColoredVertex(vec3(0.5f, -0.5f, 0.5f), vec3(0.0f, -1.0f, 0.0f), vec2(1.0f, 1.0f)),
        TexturedColoredVertex(vec3(-0.5f, -0.5f, 0.5f), vec3(0.0f, -1.0f, 0.0f), vec2(0.0f, 1.0f)),
        TexturedColoredVertex(vec3(-0.5f, -0.5f, -0.5f), vec3(0.0f, -1.0f, 0.0f), vec2(0.0f, 0.0f)),
        
        TexturedColoredVertex(vec3(-0.5f, 0.5f, 0.5f), vec3(0.0f, 0.0f, 1.0f), vec2(0.0f, 1.0f)), // near 
        TexturedColoredVertex(vec3(-0.5f, -0.5f, 0.5f), vec3(0.0f, 0.0f, 1.0f), vec2(0.0f, 0.0f)),
        TexturedColoredVertex(vec3(0.5f, -0.5f, 0.5f), vec3(0.0f, 0.0f, 1.0f), vec2(1.0f, 0.0f)),
        
        TexturedColoredVertex(vec3(0.5f, 0.5f, 0.5f), vec3(0.0f, 0.0f, 1.0f), vec2(1.0f, 1.0f)),
        TexturedColoredVertex(vec3(-0.5f, 0.5f, 0.5f), vec3(0.0f, 0.0f, 1.0f), vec2(0.0f, 1.0f)),
        TexturedColoredVertex(vec3(0.5f, -0.5f, 0.5f), vec3(0.0f, 0.0f, 1.0f), vec2(1.0f, 0.0f)),
        
        TexturedColoredVertex(vec3(0.5f, 0.5f, 0.5f), vec3(1.0f, 0.0f, 0.0f), vec2(1.0f, 1.0f)), // right 
        TexturedColoredVertex(vec3(0.5f, -0.5f, -0.5f), vec3(1.0f, 0.0f, 0.0f), vec2(0.0f, 0.0f)),
        TexturedColoredVertex(vec3(0.5f, 0.5f, -0.5f), vec3(1.0f, 0.0f, 0.0f), vec2(1.0f, 0.0f)),
        
        TexturedColoredVertex(vec3(0.5f, -0.5f, -0.5f), vec3(1.0f, 0.0f, 0.0f), vec2(0.0f, 0.0f)),
        TexturedColoredVertex(vec3(0.5f, 0.5f, 0.5f), vec3(1.0f, 0.0f, 0.0f), vec2(1.0f, 1.0f)),
        TexturedColoredVertex(vec3(0.5f, -0.5f, 0.5f), vec3(1.0f, 0.0f, 0.0f), vec2(0.0f, 1.0f)),
        
        TexturedColoredVertex(vec3(0.5f, 0.5f, 0.5f), vec3(0.0f, 1.0f, 0.0f), vec2(1.0f, 1.0f)), // top 
        TexturedColoredVertex(vec3(0.5f, 0.5f, -0.5f), vec3(0.0f, 1.0f, 0.0f), vec2(1.0f, 0.0f)),
        TexturedColoredVertex(vec3(-0.5f, 0.5f, -0.5f), vec3(0.0f, 1.0f, 0.0f), vec2(0.0f, 0.0f)),
        
        TexturedColoredVertex(vec3(0.5f, 0.5f, 0.5f), vec3(0.0f, 1.0f, 0.0f), vec2(1.0f, 1.0f)),
        TexturedColoredVertex(vec3(-0.5f, 0.5f, -0.5f), vec3(0.0f, 1.0f, 0.0f), vec2(0.0f, 0.0f)),
        TexturedColoredVertex(vec3(-0.5f, 0.5f, 0.5f), vec3(0.0f, 1.0f, 0.0f), vec2(0.0f, 1.0f))
};

void setProjectionMatrix(int shaderProgram, mat4 projectionMatrix) {
    glUseProgram(shaderProgram);
    GLuint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projection_matrix");
    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
}

void setViewMatrix(int shaderProgram, mat4 viewMatrix) {
    glUseProgram(shaderProgram);
    GLuint viewMatrixLocation = glGetUniformLocation(shaderProgram, "view_matrix");
    glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
}

void setWorldMatrix(int shaderProgram, mat4 worldMatrix) {
    glUseProgram(shaderProgram);
    GLuint worldMatrixLocation = glGetUniformLocation(shaderProgram, "model_matrix");
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &worldMatrix[0][0]);
}

int main(int argc, char *argv[]) {
    if (!InitContext()) return -1;
    
    // background
    glClearColor(0.41f, 0.44f, 0.62f, 1.0f);
    
    std::string shaderPathPrefix = "../assets/shaders/";
    
    GLuint shaderScene = loadSHADER(shaderPathPrefix + "scene_vertex.glsl", shaderPathPrefix + "scene_fragment.glsl");
    
    GLuint shaderShadow = loadSHADER(shaderPathPrefix + "shadow_vertex.glsl",
                                     shaderPathPrefix + "shadow_fragment.glsl");
    
    // Load Textures
    GLuint brickTextureID = loadTexture("../assets/textures/brick.jpg");
    GLuint cementTextureID = loadTexture("../assets/textures/cement.jpg");
    GLuint tennisTextureID = loadTexture("../assets/textures/tennisball.jpg");
    GLuint glossyTextureID = loadTexture("../assets/textures/glossy2.jpg");
    GLuint clayTextureID = loadTexture("../assets/textures/clay3.jpg");
    GLuint noTextureID = loadTexture("../assets/textures/white.jpg");
    
    glUseProgram(shaderScene);
    GLuint textureflag = glGetUniformLocation(shaderScene, "useTexture");
    GLint p_array[1];
    GLint *currentvalue = p_array;
    
    GLuint shadowflag = glGetUniformLocation(shaderScene, "useShadow");
    GLint p_arrayS[1];
    GLint *currentvalueS = p_arrayS;
    
    
    // Setup texture and framebuffer for creating shadow map
    
    // Dimensions of the shadow texture, which should cover the viewport window size and shouldn't be oversized and waste resources
    const unsigned int DEPTH_MAP_TEXTURE_SIZE = 1024;
    
    // Variable storing index to texture used for shadow mapping
    GLuint depth_map_texture;
    // Get the texture
    glGenTextures(1, &depth_map_texture);
    // Bind the texture so the next glTex calls affect it
    glBindTexture(GL_TEXTURE_2D, depth_map_texture);
    // Create the texture and specify it's attributes, including widthn height, components (only depth is stored, no color information)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, DEPTH_MAP_TEXTURE_SIZE, DEPTH_MAP_TEXTURE_SIZE, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT,
                 NULL);
    // Set texture sampler parameters.
    // The two calls below tell the texture sampler inside the shader how to upsample and downsample the texture. Here we choose the nearest filtering option, which means we just use the value of the closest pixel to the chosen image coordinate.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // The two calls below tell the texture sampler inside the shader how it should deal with texture coordinates outside of the [0, 1] range. Here we decide to just tile the image.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    
    
    // Variable storing index to framebuffer used for shadow mapping
    GLuint depth_map_fbo;  // fbo: framebuffer object
    // Get the framebuffer
    glGenFramebuffers(1, &depth_map_fbo);
    // Bind the framebuffer so the next glFramebuffer calls affect it
    glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
    // Attach the depth map texture to the depth map framebuffer
    //glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, depth_map_texture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_map_texture, 0);
    glDrawBuffer(GL_NONE); //disable rendering colors, only write depth values
    glReadBuffer(GL_NONE); //disable rendering colors, only write depth values 
    
    
    // Shader config
    
    GLuint textureMapLocation = glGetUniformLocation(shaderScene, "textureSampler");
    glUniform1i(textureMapLocation, 0);
    GLuint shadowMapLocation = glGetUniformLocation(shaderScene, "shadow_map");
    glUniform1i(shadowMapLocation, 1);
    
    
    
    // Camera parameters for view transform
    vec3 cameraPosition(0.6f, 3.0f, 10.0f);
    vec3 cameraPositionWalking(0.6f, 3.0f, 10.0f);
    vec3 cameraLookAt(0.0f, 0.0f, -1.0f);
    vec3 cameraUp(0.0f, 1.0f, 0.0f);
    
    // Other camera parameters
    float cameraSpeed = 3.0f;
    float cameraFastSpeed = 2 * cameraSpeed;
    float cameraHorizontalAngle = 90.0f;
    float cameraVerticalAngle = 0.0f;
    fov = 70.0f;
    bool cameraFirstPerson = true;
    
    int selection = 0;
    
    
    // Set projection matrix for shader, this won't change
    mat4 projectionMatrix = glm::perspective(70.0f,           // field of view in degrees
                                             WIDTH * 1.0f / HEIGHT, // aspect ratio
                                             0.01f, 800.0f);  // near and far (near > 0)

    // Set initial view matrix on both shaders
    mat4 viewMatrix = lookAt(cameraPosition,                // eye
                             cameraPosition + cameraLookAt, // center
                             cameraUp);                     // up

    // Set projection matrix on both shaders
    SetUniformMat4(shaderScene, "projection_matrix", projectionMatrix);
    
    // Set view matrix on both shaders
    SetUniformMat4(shaderScene, "view_matrix", viewMatrix);
    
    
    float lightAngleOuter = 30.0;
    float lightAngleInner = 20.0;
    // Set light cutoff angles on scene shader
    SetUniform1fValue(shaderScene, "light_cutoff_inner", cos(radians(lightAngleInner)));
    SetUniform1fValue(shaderScene, "light_cutoff_outer", cos(radians(lightAngleOuter)));
    
    // Set light color on scene shader
    SetUniformVec3(shaderScene, "light_color", vec3(1.0, 1.0, 1.0));
    
    // Set object color on scene shader
    SetUniformVec3(shaderScene, "object_color", vec3(1.0, 1.0, 1.0));
    
    int vao = createTexturedCubeVAO();
    int sphereVAO = createSphereObject();
    
    
    // For frame time
    float lastFrameTime = glfwGetTime();
    int lastMouseLeftState = GLFW_RELEASE;
    double lastMousePosX, lastMousePosY;
    glfwGetCursorPos(window, &lastMousePosX, &lastMousePosY);
    
    // Other OpenGL states to set once
    // Enable Backface culling
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
    
    glBindVertexArray(vao);
    int previousXstate = GLFW_RELEASE;
    int previousZstate = GLFW_RELEASE;
    // Entering Main Loop
    while (!glfwWindowShouldClose(window)) {
        // Frame time calculation
        float dt = glfwGetTime() - lastFrameTime;
        lastFrameTime += dt;
        
        
        // set projection matrix for fov changes
        projectionMatrix = glm::perspective(radians(fov),            // field of view in degrees
                                            800.0f / 600.0f,  // aspect ratio
                                            0.01f, 100.0f);   // near and far (near > 0)
        
        setProjectionMatrix(shaderScene, projectionMatrix);
        
        
        // light parameters
        vec3 lightPosition = vec3(1.0f, 20.0f, 5.0f); // the location of the light in 3D space
        vec3 lightFocus(0.0, -1.0, 0.0);      // the point in 3D space the light "looks" at
        vec3 lightDirection = normalize(lightFocus - lightPosition);
        
        float lightNearPlane = 1.0f;
        float lightFarPlane = 180.0f;
        
        mat4 lightProjectionMatrix = //frustum(-1.0f, 1.0f, -1.0f, 1.0f, lightNearPlane, lightFarPlane);
                perspective(20.0f, (float) DEPTH_MAP_TEXTURE_SIZE / (float) DEPTH_MAP_TEXTURE_SIZE, lightNearPlane,
                            lightFarPlane);
        mat4 lightViewMatrix = lookAt(lightPosition, lightFocus, vec3(0.0f, 1.0f, 0.0f));
        mat4 lightSpaceMatrix = lightProjectionMatrix * lightViewMatrix;
        
        // Set light space matrix on both shaders
        SetUniformMat4(shaderShadow, "light_view_proj_matrix", lightSpaceMatrix);
        SetUniformMat4(shaderScene, "light_view_proj_matrix", lightSpaceMatrix);
        
        // Set light far and near planes on scene shader
        SetUniform1fValue(shaderScene, "light_near_plane", lightNearPlane);
        SetUniform1fValue(shaderScene, "light_far_plane", lightFarPlane);
        
        // Set light position on scene shader
        SetUniformVec3(shaderScene, "light_position", lightPosition);
        
        // Set light direction on scene shader
        SetUniformVec3(shaderScene, "light_direction", lightDirection);
        
        
        // Set model matrix and send to both shaders
        mat4 modelMatrix = mat4(1.0f);
        
        SetUniformMat4(shaderScene, "model_matrix", modelMatrix);
        SetUniformMat4(shaderShadow, "model_matrix", modelMatrix);
        
        // Set the view matrix for first person camera and send to both shaders
        mat4 viewMatrix = lookAt(cameraPosition, cameraPosition + cameraLookAt, cameraUp);
        SetUniformMat4(shaderScene, "view_matrix", viewMatrix);
        
        // Set view position on scene shader
        SetUniformVec3(shaderScene, "view_position", cameraPosition);
        
        partMatrix = scale(mat4(1.0f), vec3(-1.0f, 1.0f, 1.0f));
        groupMatrix = translate(mat4(1.0f), vec3(0.0f + position.x, 0.0f + position.y, 0.0f + position.z)) *
                      scale(mat4(1.0f), vec3(0.1f + scaling.x, 0.1f + scaling.y, 0.1f + scaling.z)) *
                      rotate(mat4(1.0f), radians(0.0f + rotation), vec3(0.0f, 1.0f, 0.0f));
        worldMatrix = groupMatrix * partMatrix;
        
        
        // Render shadow in 2 passes: 1- Render depth map, 2- Render scene
        // 1- Render shadow map:
        // a- use program for shadows
        // b- resize window coordinates to fix depth map output size
        // c- bind depth map framebuffer to output the depth values
        {
            // Use proper shader
            glUseProgram(shaderShadow);
            // Use proper image output size
            glViewport(0, 0, DEPTH_MAP_TEXTURE_SIZE, DEPTH_MAP_TEXTURE_SIZE);
            // Bind depth map texture as output framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
            // Clear depth data on the framebuffer
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            
            GLuint worldMatrixLocation = glGetUniformLocation(shaderShadow, "model_matrix");
            
            // Bind geometry
            glBindVertexArray(vao);
            
            renderScene(shaderShadow, vao, sphereVAO, tennisTextureID, glossyTextureID, clayTextureID, noTextureID);
            
            // Unbind geometry
            glBindVertexArray(0);
        }
        
        
        //2- Render scene: a- bind the default framebuffer and b- just render like what we do normally
        {
            // Use proper shader
            glUseProgram(shaderScene);
            // Use proper image output size
            // Side note: we get the size from the framebuffer instead of using WIDTH and HEIGHT because of a bug with highDPI displays
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            glViewport(0, 0, width, height);
            // Bind screen as output framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            // Clear color and depth data on framebuffer
            glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            // Bind depth map texture: not needed, by default it is active
            //glActiveTexture(GL_TEXTURE0);
            
            
            // Draw textured geometry
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, depth_map_texture);
            
            // Draw textured geometry
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, brickTextureID);
            // Bind geometry
            
            GLuint worldMatrixLocation = glGetUniformLocation(shaderScene, "model_matrix");
            // Bind geometry
            glBindVertexArray(vao);
            
            renderScene(shaderScene, vao, sphereVAO, tennisTextureID, glossyTextureID, clayTextureID, noTextureID);
            
            // Unbind geometry
            glBindVertexArray(0);
        }
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        
        // Handle inputs
        
        // Escape
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        
        
        // Toggle texture 
        if (previousXstate == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
            glGetUniformiv(shaderScene, textureflag, currentvalue);
            if (currentvalue[0] == 1) {
                glUseProgram(shaderScene);
                glUniform1i(textureflag, 0);
            } else {
                
                glUseProgram(shaderScene);
                glUniform1i(textureflag, 1);
            }
        }
        previousXstate = glfwGetKey(window, GLFW_KEY_X);
        
        
        // Toggle shadow
        if (previousZstate == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
            glGetUniformiv(shaderScene, shadowflag, currentvalueS);
            if (currentvalueS[0] == 1) {
                glUseProgram(shaderScene);
                glUniform1i(shadowflag, 0);
            } else {
                
                glUseProgram(shaderScene);
                glUniform1i(shadowflag, 1);
            }
        }
        previousZstate = glfwGetKey(window, GLFW_KEY_Z);
        
        
        double mousePosX, mousePosY;
        glfwGetCursorPos(window, &mousePosX, &mousePosY);
        
        double dx = mousePosX - lastMousePosX;
        double dy = mousePosY - lastMousePosY;
        
        lastMousePosX = mousePosX;
        lastMousePosY = mousePosY;
        
        // Convert to spherical coordinates
        const float cameraAngularSpeed = 60.0f;
        cameraHorizontalAngle -= dx * cameraAngularSpeed * dt;
        cameraVerticalAngle -= dy * cameraAngularSpeed * dt;
        
        // Clamp vertical angle to [-85, 85] degrees
        cameraVerticalAngle = std::max(-85.0f, std::min(85.0f, cameraVerticalAngle));
        if (cameraHorizontalAngle > 360) {
            cameraHorizontalAngle -= 360;
        } else if (cameraHorizontalAngle < -360) {
            cameraHorizontalAngle += 360;
        }
        
        float theta = radians(cameraHorizontalAngle);
        float phi = radians(cameraVerticalAngle);
        
        // Number selections
        if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
            selection = 0;
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
            selection = 1;
        
        
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) // close window
            glfwSetWindowShouldClose(window, true);
        
        
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) && dx > 0) // moving left, zoom in
        {
            fov += 1.0f;
        }
        
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) && dx < 0) // moving right, zoom out
        {
            fov -= 1.0f;
        }
        
        
        if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) // scale object up
        {
            scaling.x += 0.1f;
            scaling.y += 0.1f;
            scaling.z += 0.1f;
        }
        
        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) // scale object down
        {
            scaling.x -= 0.1f;
            scaling.y -= 0.1f;
            scaling.z -= 0.1f;
        }
        
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) // move object to the left
        {
            position.x -= 0.1f;
            
        }
        
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) // move object to the right
        {
            position.x += 0.1f;
        }
        
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) // move object down
        {
            position.y -= 0.1f;
        }
        
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) // move object up
        {
            position.y += 0.1f;
        }
        
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) // rotate object to the left
        {
            rotation += 5.0f;
        }
        
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) // rotate object to the right
        {
            rotation -= 5.0f;
        }
        
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) // reset object back to origin
        {
            position.x = 0.0f;
            position.y = 0.0f;
            position.z = 0.0f;
            
            scaling.x = 1.0f;
            scaling.y = 1.0f;
            scaling.z = 1.0f;
            rotation = 0.0f;
            
        }
        
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) // zoom out
        {
            fov += 1.0f;
        }
        
        
        if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) // zoom in
        {
            fov -= 1.0f;
        }
        
        // Free camera, default
        if (selection == 0) {
            
            cameraPosition = cameraPositionWalking;
            
            bool fastCam = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                           glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
            float currentCameraSpeed = (fastCam) ? cameraFastSpeed : cameraSpeed;
            
            double mousePosX, mousePosY;
            glfwGetCursorPos(window, &mousePosX, &mousePosY);
            
            double dx = mousePosX - lastMousePosX;
            double dy = mousePosY - lastMousePosY;
            
            lastMousePosX = mousePosX;
            lastMousePosY = mousePosY;
            
            // Convert to spherical coordinates
            const float cameraAngularSpeed = 60.0f;
            cameraHorizontalAngle -= dx * cameraAngularSpeed * dt;
            cameraVerticalAngle -= dy * cameraAngularSpeed * dt;
            
            // Clamp vertical angle to [-85, 85] degrees
            cameraVerticalAngle = std::max(-85.0f, std::min(85.0f, cameraVerticalAngle));
            if (cameraHorizontalAngle > 360) {
                cameraHorizontalAngle -= 360;
            } else if (cameraHorizontalAngle < -360) {
                cameraHorizontalAngle += 360;
            }
            
            float theta = radians(cameraHorizontalAngle);
            float phi = radians(cameraVerticalAngle);
            
            cameraLookAt = vec3(cosf(phi) * cosf(theta), sinf(phi), -cosf(phi) * sinf(theta));
            vec3 cameraSideVector = glm::cross(cameraLookAt, vec3(0.0f, 1.0f, 0.0f));
            
            glm::normalize(cameraSideVector);
            
            
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) // move camera to the left
            {
                cameraPosition -= cameraSideVector * currentCameraSpeed * dt;
            }
            
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) // move camera to the right
            {
                cameraPosition += cameraSideVector * currentCameraSpeed * dt;
            }
            
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) // move camera up
            {
                cameraPosition -= cameraLookAt * currentCameraSpeed * dt;
            }
            
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) // move camera down
            {
                cameraPosition += cameraLookAt * currentCameraSpeed * dt;
            }
            
            
            viewMatrix = mat4(1.0);
            
            if (cameraFirstPerson) {
                viewMatrix = lookAt(cameraPosition, cameraPosition + cameraLookAt, cameraUp);
            } else {
                float radius = 5.0f;
                glm::vec3 position = cameraPosition - radius * cameraLookAt;
                viewMatrix = lookAt(position, position + cameraLookAt, cameraUp);
            }
            
            // Keep track of walking position for when camera is changed to birds eye
            cameraPositionWalking = cameraPosition;
        }
        // Birds eye camera
        if (selection == 1) {
            
            cameraPosition = vec3(0.6f, 10.0f, 10.0f);
            
            bool fastCam = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                           glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
            float currentCameraSpeed = (fastCam) ? cameraFastSpeed : cameraSpeed;
            
            double mousePosX, mousePosY;
            glfwGetCursorPos(window, &mousePosX, &mousePosY);
            
            double dx = mousePosX - lastMousePosX;
            double dy = mousePosY - lastMousePosY;
            
            lastMousePosX = mousePosX;
            lastMousePosY = mousePosY;
            
            // Convert to spherical coordinates
            const float cameraAngularSpeed = 60.0f;
            cameraHorizontalAngle -= dx * cameraAngularSpeed * dt;
            cameraVerticalAngle -= dy * cameraAngularSpeed * dt;
            
            // Clamp vertical angle to [-85, 85] degrees
            cameraVerticalAngle = std::max(-85.0f, std::min(85.0f, cameraVerticalAngle));
            if (cameraHorizontalAngle > 360) {
                cameraHorizontalAngle -= 360;
            } else if (cameraHorizontalAngle < -360) {
                cameraHorizontalAngle += 360;
            }
            
            float theta = radians(cameraHorizontalAngle);
            float phi = radians(cameraVerticalAngle);
            
            cameraLookAt = vec3(cosf(phi) * cosf(theta), sinf(phi), -cosf(phi) * sinf(theta));
            vec3 cameraSideVector = glm::cross(cameraLookAt, vec3(0.0f, 1.0f, 0.0f));
            
            glm::normalize(cameraSideVector);
            
            viewMatrix = mat4(1.0);
            
            if (cameraFirstPerson) {
                viewMatrix = lookAt(cameraPosition, cameraPosition + cameraLookAt, cameraUp);
            } else {
                float radius = 5.0f;
                glm::vec3 position = cameraPosition - radius * cameraLookAt;
                viewMatrix = lookAt(position, position + cameraLookAt, cameraUp);
            }
        }
        
    }
    
    
    glfwTerminate();
    
    return 0;
}


bool InitContext() {
    // Initialize GLFW and OpenGL version
    glfwInit();

#if defined(__APPLE__)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    // On windows, we set OpenGL version to 2.1, to support more hardware
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#endif
    
    // Create Window and rendering context using GLFW, resolution is 800x600
    window = glfwCreateWindow(WIDTH, HEIGHT, "Comp371 - Lab 08", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwMakeContextCurrent(window);
    
    
    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to create GLEW" << std::endl;
        glfwTerminate();
        return false;
    }
    
    return true;
}

GLuint loadTexture(const char *filename) {
    // Step1 Create and bind textures
    GLuint textureId = 0;
    glGenTextures(1, &textureId);
    assert(textureId != 0);
    
    
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    // Step2 Set filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Step3 Load Textures with dimension data
    int width, height, nrChannels;
    unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (!data) {
        std::cerr << "Error::Texture could not load texture file:" << filename << std::endl;
        return 0;
    }
    
    // Step4 Upload the texture to the PU
    GLenum format = 0;
    if (nrChannels == 1)
        format = GL_RED;
    else if (nrChannels == 3)
        format = GL_RGB;
    else if (nrChannels == 4)
        format = GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height,
                 0, format, GL_UNSIGNED_BYTE, data);
    
    // Step5 Free resources
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return textureId;
}

int createTexturedCubeVAO() {
    // Create a vertex array
    GLuint vertexArrayObject;
    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);
    
    // Upload Vertex Buffer to the GPU, keep a reference to it (vertexBufferObject)
    GLuint vertexBufferObject;
    glGenBuffers(1, &vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texturedCubeVertexArray), texturedCubeVertexArray, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0,                   // attribute 0 matches aPos in Vertex Shader
                          3,                   // size
                          GL_FLOAT,            // type
                          GL_FALSE,            // normalized?
                          sizeof(TexturedColoredVertex), // stride - each vertex contain 2 vec3 (position, color)
                          (void *) 0             // array buffer offset
    );
    glEnableVertexAttribArray(0);
    
    
    glVertexAttribPointer(1,                            // attribute 1 matches aColor in Vertex Shader
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(TexturedColoredVertex),
                          (void *) sizeof(vec3)      // color is offseted a vec3 (comes after position)
    );
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2,                            // attribute 2 matches aUV in Vertex Shader
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(TexturedColoredVertex),
                          (void *) (2 * sizeof(vec3))      // uv is offseted by 2 vec3 (comes after position and color)
    );
    glEnableVertexAttribArray(2);
    
    return vertexArrayObject;
}


int createSphereObject() {
    // A vertex is a point on a polygon, it contains positions and other data (eg: colors)
    unsigned int sphereVAO;
    glGenVertexArrays(1, &sphereVAO);
    
    unsigned int vbo, ebo;
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> uv;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;
    std::vector<glm::vec3> colors;
    
    const unsigned int X_SEGMENTS = 64;
    const unsigned int Y_SEGMENTS = 64;
    const float PI = 3.14159265359;
    for (unsigned int y = 0; y <= Y_SEGMENTS; ++y) {
        for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
            float xSegment = (float) x / (float) X_SEGMENTS;
            float ySegment = (float) y / (float) Y_SEGMENTS;
            float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            float yPos = std::cos(ySegment * PI);
            float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            
            positions.push_back(glm::vec3(xPos, yPos, zPos));
            colors.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
            uv.push_back(glm::vec2(xSegment, ySegment));
            normals.push_back(glm::vec3(xPos, yPos, zPos));
        }
    }
    
    bool oddRow = false;
    for (unsigned int y = 0; y < Y_SEGMENTS; ++y) {
        if (!oddRow) // even rows: y == 0, y == 2; and so on
        {
            for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
                indices.push_back(y * (X_SEGMENTS + 1) + x);
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
            }
        } else {
            for (int x = X_SEGMENTS; x >= 0; --x) {
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                indices.push_back(y * (X_SEGMENTS + 1) + x);
            }
        }
        oddRow = !oddRow;
    }
    indexCount = indices.size();
    
    std::vector<float> data;
    for (unsigned int i = 0; i < positions.size(); ++i) {
        data.push_back(positions[i].x);
        data.push_back(positions[i].y);
        data.push_back(positions[i].z);
        if (colors.size() > 0) {
            data.push_back(colors[i].x);
            data.push_back(colors[i].y);
            data.push_back(colors[i].z);
        }
        if (uv.size() > 0) {
            data.push_back(uv[i].x);
            data.push_back(uv[i].y);
        }
        if (normals.size() > 0) {
            data.push_back(normals[i].x);
            data.push_back(normals[i].y);
            data.push_back(normals[i].z);
        }
    }
    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
    float stride = (3 + 2 + 3 + 3) * sizeof(float);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void *) (6 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void *) (8 * sizeof(float)));
    
    glBindBuffer(GL_ARRAY_BUFFER, 0); // VAO already stored the state we just defined, safe to unbind buffer
    glBindVertexArray(0); // Unbind to not modify the VAO
    
    return sphereVAO;
}


void renderScene(GLuint shader, int texturedCubeVAO, int sphereVAO, GLuint tennisTextureID, GLuint glossyTextureID,
                 GLuint clayTextureID, GLuint noTextureID) {
    
    glBindTexture(GL_TEXTURE_2D, noTextureID); // no texture
    
    // Draw 100x100 ground grid
    for (int i = -50; i < 51; ++i) {
        mat4 groundWorldMatrix =
                translate(mat4(1.0f), vec3(0.0f + i, -0.01f, 0.0f)) * scale(mat4(1.0f), vec3(0.05f, 0.0f, 100.0f));
        worldMatrix = groundWorldMatrix;
        setWorldMatrix(shader, worldMatrix);
        SetUniformVec3(shader, "object_color", vec3(1.0f, 1.0f, 0.0f)); // Yellow
        glDrawArrays(GL_TRIANGLES, 0, 36); // 36 vertices, starting at index 0
    }
    
    for (int j = -50; j < 51; ++j) {
        mat4 groundWorldMatrix =
                translate(mat4(1.0f), vec3(0.0f, -0.01f, 0.0f + j)) * scale(mat4(1.0f), vec3(100.0f, 0.0f, 0.05f));
        worldMatrix = groundWorldMatrix;
        setWorldMatrix(shader, worldMatrix);
        SetUniformVec3(shader, "object_color", vec3(1.0f, 1.0f, 0.0f)); // Yellow
        glDrawArrays(GL_TRIANGLES, 0, 36); // 36 vertices, starting at index 0
    }
    
    // Floor
    mat4 courtWorldMatrix =
            translate(mat4(1.0f), vec3(0.0f, -0.09f, 0.0f)) * scale(mat4(1.0f), vec3(100.0f, 0.1f, 100.0f));
    glBindTexture(GL_TEXTURE_2D, clayTextureID);
    worldMatrix = courtWorldMatrix;
    setWorldMatrix(shader, worldMatrix);
    SetUniformVec3(shader, "object_color", vec3(0.38f, 0.63f, 0.33f)); // Green
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    
    glBindTexture(GL_TEXTURE_2D, noTextureID); // no texture
    // Draw tree
    // Trunk
    mat4 treeWorldMatrix = translate(mat4(1.0f), vec3(5.0f, 2.5f, 0.0f)) * scale(mat4(1.0f), vec3(1.0f, 5.0f, 1.0f));
    worldMatrix = groupMatrix * treeWorldMatrix;
    setWorldMatrix(shader, worldMatrix);
    SetUniformVec3(shader, "object_color", vec3(0.33f, 0.2f, 0.05f)); // Brown
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    // Leaves
    treeWorldMatrix = translate(mat4(1.0f), vec3(5.0f, 5.0f, 0.0f)) * scale(mat4(1.0f), vec3(4.0f, 3.0f, 4.0f));
    worldMatrix = groupMatrix * treeWorldMatrix;
    setWorldMatrix(shader, worldMatrix);
    SetUniformVec3(shader, "object_color", vec3(0.18f, 0.33f, 0.15f)); // Green
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    
    // Draw coordinate axis
    // X axis
    mat4 axisWorldMatrix = translate(mat4(1.0f), vec3(1.5f, 0.0f, 0.0f)) * scale(mat4(1.0f), vec3(3.0f, 0.1f, 0.1f));
    worldMatrix = axisWorldMatrix;
    setWorldMatrix(shader, worldMatrix);
    SetUniformVec3(shader, "object_color", vec3(1.0f, 0.0f, 0.0f)); // Blue
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    // Y axis
    axisWorldMatrix = translate(mat4(1.0f), vec3(0.0f, 1.5f, 0.0f)) * scale(mat4(1.0f), vec3(0.1f, 3.0f, 0.1f));
    worldMatrix = axisWorldMatrix;
    setWorldMatrix(shader, worldMatrix);
    SetUniformVec3(shader, "object_color", vec3(0.0f, 1.0f, 0.0f)); // Green
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    // Z axis
    axisWorldMatrix = translate(mat4(1.0f), vec3(0.0f, 0.0f, 1.5f)) * scale(mat4(1.0f), vec3(0.1f, 0.1f, 3.0f));
    worldMatrix = axisWorldMatrix;
    setWorldMatrix(shader, worldMatrix);
    SetUniformVec3(shader, "object_color", vec3(0.0f, 0.0f, 1.0f)); // Red
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    
}