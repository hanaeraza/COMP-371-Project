
#include <iostream>
#include <list>
#include <algorithm>
#include <vector>
#include <map>
#include <random>
#include <cmath>

#define GLEW_STATIC 1   // This allows linking with Static Library on Windows, without DLL
#include <GL/glew.h>    // Include GLEW - OpenGL Extension Wrangler
#include <GLFW/glfw3.h> // GLFW provides a cross-platform interface for creating a graphical context,
// initializing OpenGL and binding inputs
#include <glm/glm.hpp>  // GLM is an optimized math library with syntax to similar to OpenGL Shading Language
#include <glm/gtc/matrix_transform.hpp> // include this to create transformation matrices
#include <glm/common.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>

using namespace glm;
using namespace std;

#if defined(__APPLE__)
std::string pathPrefix = "../";
#else
string pathPrefix = "";
#endif




unsigned int indexCount;

GLuint loadTexture(const char* filename);

int createSphereObject();

const char* getLightVertexShaderSource();

const char* getLightFragmentShaderSource();

//const char* getShadowVertexShaderSource();

//const char* getShadowFragmentShaderSource();

float rotX = 0.0f;
int camNum = 3;
int carLight = 0;

// window dimensions
const GLuint WIDTH = 1024, HEIGHT = 768;

int compileAndLinkShaders(const char* vertexShaderSource, const char* fragmentShaderSource);

GLFWwindow* window = nullptr;
bool InitContext();

struct TexturedColoredVertex
{
    TexturedColoredVertex(vec3 _position, vec3 _color, vec2 _uv)
            : position(_position), color(_color), uv(_uv) {}
    
    vec3 position;
    vec3 color;
    vec2 uv;
};

// Textured Cube model
const TexturedColoredVertex texturedCubeVertexArray[] = {  // position,                            color
        TexturedColoredVertex(vec3(-0.5f,-0.5f,-0.5f), vec3(-1.0f,  0.0f,  0.0f), vec2(0.0f, 0.0f)), //left - red
        TexturedColoredVertex(vec3(-0.5f,-0.5f, 0.5f), vec3(-1.0f,  0.0f,  0.0f), vec2(0.0f, 1.0f)),
        TexturedColoredVertex(vec3(-0.5f, 0.5f, 0.5f), vec3(-1.0f,  0.0f,  0.0f), vec2(1.0f, 1.0f)),
        
        TexturedColoredVertex(vec3(-0.5f,-0.5f,-0.5f), vec3(-1.0f,  0.0f,  0.0f), vec2(0.0f, 0.0f)),
        TexturedColoredVertex(vec3(-0.5f, 0.5f, 0.5f), vec3(-1.0f,  0.0f,  0.0f), vec2(1.0f, 1.0f)),
        TexturedColoredVertex(vec3(-0.5f, 0.5f,-0.5f), vec3(-1.0f,  0.0f,  0.0f), vec2(1.0f, 0.0f)),
        
        TexturedColoredVertex(vec3(0.5f, 0.5f,-0.5f), vec3(0.0f,  0.0f, -1.0f), vec2(1.0f, 1.0f)), // far - blue
        TexturedColoredVertex(vec3(-0.5f,-0.5f,-0.5f), vec3(0.0f,  0.0f, -1.0f), vec2(0.0f, 0.0f)),
        TexturedColoredVertex(vec3(-0.5f, 0.5f,-0.5f), vec3(0.0f,  0.0f, -1.0f), vec2(0.0f, 1.0f)),
        
        TexturedColoredVertex(vec3(0.5f, 0.5f,-0.5f), vec3(0.0f,  0.0f, -1.0f), vec2(1.0f, 1.0f)),
        TexturedColoredVertex(vec3(0.5f,-0.5f,-0.5f), vec3(0.0f,  0.0f, -1.0f), vec2(1.0f, 0.0f)),
        TexturedColoredVertex(vec3(-0.5f,-0.5f,-0.5f), vec3(0.0f,  0.0f, -1.0f), vec2(0.0f, 0.0f)),
        
        TexturedColoredVertex(vec3(0.5f,-0.5f, 0.5f), vec3(0.0f, -1.0f,  0.0f), vec2(1.0f, 1.0f)), // bottom - turquoise
        TexturedColoredVertex(vec3(-0.5f,-0.5f,-0.5f), vec3(0.0f, -1.0f,  0.0f), vec2(0.0f, 0.0f)),
        TexturedColoredVertex(vec3(0.5f,-0.5f,-0.5f), vec3(0.0f, -1.0f,  0.0f), vec2(1.0f, 0.0f)),
        
        TexturedColoredVertex(vec3(0.5f,-0.5f, 0.5f), vec3(0.0f, -1.0f,  0.0f), vec2(1.0f, 1.0f)),
        TexturedColoredVertex(vec3(-0.5f,-0.5f, 0.5f), vec3(0.0f, -1.0f,  0.0f), vec2(0.0f, 1.0f)),
        TexturedColoredVertex(vec3(-0.5f,-0.5f,-0.5f), vec3(0.0f, -1.0f,  0.0f), vec2(0.0f, 0.0f)),
        
        TexturedColoredVertex(vec3(-0.5f, 0.5f, 0.5f), vec3(0.0f,  0.0f,  1.0f), vec2(0.0f, 1.0f)), // near - green
        TexturedColoredVertex(vec3(-0.5f,-0.5f, 0.5f), vec3(0.0f,  0.0f,  1.0f), vec2(0.0f, 0.0f)),
        TexturedColoredVertex(vec3(0.5f,-0.5f, 0.5f), vec3(0.0f,  0.0f,  1.0f), vec2(1.0f, 0.0f)),
        
        TexturedColoredVertex(vec3(0.5f, 0.5f, 0.5f), vec3(0.0f,  0.0f,  1.0f), vec2(1.0f, 1.0f)),
        TexturedColoredVertex(vec3(-0.5f, 0.5f, 0.5f), vec3(0.0f,  0.0f,  1.0f), vec2(0.0f, 1.0f)),
        TexturedColoredVertex(vec3(0.5f,-0.5f, 0.5f), vec3(0.0f,  0.0f,  1.0f), vec2(1.0f, 0.0f)),
        
        TexturedColoredVertex(vec3(0.5f, 0.5f, 0.5f), vec3(1.0f,  0.0f,  0.0f), vec2(1.0f, 1.0f)), // right - purple
        TexturedColoredVertex(vec3(0.5f,-0.5f,-0.5f), vec3(1.0f,  0.0f,  0.0f), vec2(0.0f, 0.0f)),
        TexturedColoredVertex(vec3(0.5f, 0.5f,-0.5f), vec3(1.0f,  0.0f,  0.0f), vec2(1.0f, 0.0f)),
        
        TexturedColoredVertex(vec3(0.5f,-0.5f,-0.5f), vec3(1.0f,  0.0f,  0.0f), vec2(0.0f, 0.0f)),
        TexturedColoredVertex(vec3(0.5f, 0.5f, 0.5f), vec3(1.0f,  0.0f,  0.0f), vec2(1.0f, 1.0f)),
        TexturedColoredVertex(vec3(0.5f,-0.5f, 0.5f), vec3(1.0f,  0.0f,  0.0f), vec2(0.0f, 1.0f)),
        
        TexturedColoredVertex(vec3(0.5f, 0.5f, 0.5f), vec3(0.0f,  1.0f,  0.0f), vec2(1.0f, 1.0f)), // top - yellow
        TexturedColoredVertex(vec3(0.5f, 0.5f,-0.5f), vec3(0.0f,  1.0f,  0.0f), vec2(1.0f, 0.0f)),
        TexturedColoredVertex(vec3(-0.5f, 0.5f,-0.5f), vec3(0.0f,  1.0f,  0.0f), vec2(0.0f, 0.0f)),
        
        TexturedColoredVertex(vec3(0.5f, 0.5f, 0.5f), vec3(0.0f,  1.0f,  0.0f), vec2(1.0f, 1.0f)),
        TexturedColoredVertex(vec3(-0.5f, 0.5f,-0.5f), vec3(0.0f,  1.0f,  0.0f), vec2(0.0f, 0.0f)),
        TexturedColoredVertex(vec3(-0.5f, 0.5f, 0.5f), vec3(0.0f,  1.0f,  0.0f), vec2(0.0f, 1.0f))
};

int createTexturedCubeVertexArrayObject();

int createLightCubeVertexArrayObject();

// shader variable setters
void SetUniformMat4(GLuint shader_id, const char* uniform_name, mat4 uniform_value)
{
    glUseProgram(shader_id);
    glUniformMatrix4fv(glGetUniformLocation(shader_id, uniform_name), 1, GL_FALSE, &uniform_value[0][0]);
}

void SetUniformVec3(GLuint shader_id, const char* uniform_name, vec3 uniform_value)
{
    glUseProgram(shader_id);
    glUniform3fv(glGetUniformLocation(shader_id, uniform_name), 1, value_ptr(uniform_value));
}

template <class T>
void SetUniform1Value(GLuint shader_id, const char* uniform_name, T uniform_value)
{
    glUseProgram(shader_id);
    glUniform1i(glGetUniformLocation(shader_id, uniform_name), uniform_value);
    glUseProgram(0);
}

void SetUniformfvalue(GLuint shader_id, const char* uniform_name, float uniform_value)
{
    glUseProgram(shader_id);
    glUniform1f(glGetUniformLocation(shader_id, uniform_name), uniform_value);
    glUseProgram(0);
}



//Draws the ground ln the right and left of the road
void drawGround(GLuint shader_id, float z, GLuint texture) {
    glBindTexture(GL_TEXTURE_2D, texture);
    glUseProgram(shader_id);
    //Drawing dirt "forest" around the road (right, left)
    mat4 groundWorldMatrix = translate(mat4(1.0f), vec3(56.0f, 0.0f, 0.0f + z)) * scale(mat4(1.0f), vec3(100.0f, 0.0f, 300.0f));
    SetUniformVec3(shader_id, "object_color", vec3(0.0, 1.0, 0.0f));
    SetUniformMat4(shader_id, "modelMatrix", groundWorldMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    groundWorldMatrix = translate(mat4(1.0f), vec3(-56.0f, 0.0f, 0.0f + z)) * scale(mat4(1.0f), vec3(100.0f, 0.0f, 300.0f));
    SetUniformVec3(shader_id, "object_color", vec3(0.0, 1.0, 0.0f));
    SetUniformMat4(shader_id, "modelMatrix", groundWorldMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}


//Draws the road
void drawRoad(GLuint shader_id, float z, GLuint text) {
    //Draw the road
    glBindTexture(GL_TEXTURE_2D, text);
    mat4 groundWorldMatrix = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f + z)) * scale(mat4(1.0f), vec3(12.0f, 0.0f, 300.0f));
    glBindTexture(GL_TEXTURE_2D, text);
    SetUniformVec3(shader_id, "object_color", vec3(0.5f, 0.5f, 0.5f));
    SetUniformMat4(shader_id, "modelMatrix", groundWorldMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 36); // 36 vertices, starting at index 0
}

void drawCar(GLuint shader_id, int vaos, vec3 carMove, GLuint carText, GLuint tireText) {
    glBindTexture(GL_TEXTURE_2D, carText);
    
    mat4 car;
    float sizeInc = 1;//make car dif size
    mat4 reposition = translate(mat4(1.0f), vec3(-2.25, 0.5, 10.0f));//position car in scene
    
    mat4 body = translate(mat4(1.0f), sizeInc*vec3(2.25f+carMove.x, 1.0f, -5.0f + carMove.z)) * scale(mat4(1.0f), sizeInc*vec3(4.0f, 1.5f, 8.0f));
    car = reposition * body;
    SetUniformVec3(shader_id, "object_color", vec3(255 / 255.0, 105 / 255.0, 180 / 255.0));
    SetUniformMat4(shader_id, "modelMatrix", car);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    mat4 light1 = translate(mat4(1.0f), sizeInc*vec3(1 + carMove.x, 1.0f, -9.0f + carMove.z)) * scale(mat4(1.0f), sizeInc*vec3(0.5f, 0.5f, 0.1f));
    car = reposition * light1;
    SetUniformVec3(shader_id, "object_color", vec3(0, 1, 1));
    SetUniformMat4(shader_id, "modelMatrix", car);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    mat4 light2 = translate(mat4(1.0f), sizeInc*vec3(3.5f + carMove.x, 1.0f, -9.0f + carMove.z)) * scale(mat4(1.0f), sizeInc*vec3(0.5f, 0.5f, 0.1f));
    car = reposition * light2;
    SetUniformVec3(shader_id, "object_color", vec3(0, 1, 1));
    SetUniformMat4(shader_id, "modelMatrix", car);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    mat4 side1 = translate(mat4(1.0f), sizeInc*vec3(0.75f + carMove.x, 2.5, -3 + carMove.z)) * scale(mat4(1.0f), sizeInc*vec3(0.1f, 1.75, 3));
    car = reposition * side1;
    SetUniformVec3(shader_id, "object_color", vec3(255 / 255.0, 105 / 255.0, 180 / 255.0));
    SetUniformMat4(shader_id, "modelMatrix", car);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    mat4 side1_2 = translate(mat4(1.0f), sizeInc*vec3(0.75f + carMove.x, 3.23, -6 + carMove.z)) * scale(mat4(1.0f), sizeInc*vec3(0.1f, 0.3, 3));
    car = reposition * side1_2;
    SetUniformVec3(shader_id, "object_color", vec3(255 / 255.0, 105 / 255.0, 180 / 255.0));
    SetUniformMat4(shader_id, "modelMatrix", car);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    mat4 side1_3 = translate(mat4(1.0f), sizeInc*vec3(0.75f + carMove.x, 1.75, -6 + carMove.z)) * scale(mat4(1.0f), sizeInc*vec3(0.1f, 0.3, 3));
    car = reposition * side1_3;
    SetUniformVec3(shader_id, "object_color", vec3(255 / 255.0, 105 / 255.0, 180 / 255.0));
    SetUniformMat4(shader_id, "modelMatrix", car);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    mat4 side1_4 = translate(mat4(1.0f), sizeInc*vec3(0.75f + carMove.x, 2.5, -7 + carMove.z)) * scale(mat4(1.0f), sizeInc*vec3(0.15f, 1.3, 1));
    car = reposition * side1_4;
    SetUniformVec3(shader_id, "object_color", vec3(255 / 255.0, 105 / 255.0, 180 / 255.0));
    SetUniformMat4(shader_id, "modelMatrix", car);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    mat4 side2 = translate(mat4(1.0f), sizeInc*vec3(3.75f + carMove.x, 2.5, -3 + carMove.z)) * scale(mat4(1.0f), sizeInc*vec3(0.1f, 1.75, 3));
    car = reposition * side2;
    SetUniformVec3(shader_id, "object_color", vec3(255 / 255.0, 105 / 255.0, 180 / 255.0));
    SetUniformMat4(shader_id, "modelMatrix", car);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    mat4 side2_2 = translate(mat4(1.0f), sizeInc*vec3(3.75f + carMove.x, 3.23, -6 + carMove.z)) * scale(mat4(1.0f), sizeInc*vec3(0.1f, 0.3, 3));
    car = reposition * side2_2;
    SetUniformVec3(shader_id, "object_color", vec3(255 / 255.0, 105 / 255.0, 180 / 255.0));
    SetUniformMat4(shader_id, "modelMatrix", car);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    mat4 side2_3 = translate(mat4(1.0f), sizeInc*vec3(3.75f + carMove.x, 1.75, -6 + carMove.z)) * scale(mat4(1.0f), sizeInc*vec3(0.1f, 0.3, 3));
    car = reposition * side2_3;
    SetUniformVec3(shader_id, "object_color", vec3(255 / 255.0, 105 / 255.0, 180 / 255.0));
    SetUniformMat4(shader_id, "modelMatrix", car);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    mat4 side2_4 = translate(mat4(1.0f), sizeInc*vec3(3.75f + carMove.x, 2.5, -7 + carMove.z)) * scale(mat4(1.0f), sizeInc*vec3(0.15f, 1.3, 1));
    car = reposition * side2_4;
    SetUniformVec3(shader_id, "object_color", vec3(255 / 255.0, 105 / 255.0, 180 / 255.0));
    SetUniformMat4(shader_id, "modelMatrix", car);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    mat4 window1 = translate(mat4(1.0f), sizeInc*vec3(2.25f + carMove.x, 1.75, -7.5 + carMove.z)) * scale(mat4(1.0f), sizeInc*vec3(3, 0.3, 0.1f));
    car = reposition * window1;
    SetUniformVec3(shader_id, "object_color", vec3(1,0,0));
    SetUniformMat4(shader_id, "modelMatrix", car);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    mat4 window2 = translate(mat4(1.0f), sizeInc*vec3(2.25f + carMove.x, 3.25, -7.5 + carMove.z)) * scale(mat4(1.0f), sizeInc*vec3(3, 0.3, 0.1f));
    car = reposition * window2;
    SetUniformVec3(shader_id, "object_color", vec3(1, 0, 0));
    SetUniformMat4(shader_id, "modelMatrix", car);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    mat4 back = translate(mat4(1.0f), sizeInc*vec3(2.25f + carMove.x, 2.5, -1.5 + carMove.z)) * scale(mat4(1.0f), sizeInc*vec3(3, 1.75, 0.1f));
    car = reposition * back;
    SetUniformVec3(shader_id, "object_color", vec3(1, 0, 0));
    SetUniformMat4(shader_id, "modelMatrix", car);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    mat4 top = translate(mat4(1.0f), sizeInc*vec3(2.25f + carMove.x, 3.4, -4.5 + carMove.z)) * scale(mat4(1.0f), sizeInc*vec3(3, 0.1, 6.0f));
    car = reposition * top;
    SetUniformVec3(shader_id, "object_color", vec3(1, 0, 1));
    SetUniformMat4(shader_id, "modelMatrix", car);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    glBindTexture(GL_TEXTURE_2D, tireText);
    glBindVertexArray(vaos);
    mat4 wheel1 = translate(mat4(1.0f), sizeInc*vec3(4.5f + carMove.x, 0.5f, -7.0f + carMove.z))* rotate(mat4(1.0f), radians(rotX), sizeInc*vec3(1, 0, 0))*scale(mat4(1.0f), sizeInc*vec3(0.5f, 1.0f, 1.0f));
    car = reposition * wheel1;
    SetUniformVec3(shader_id, "object_color", vec3(50 / 255.0, 50 / 255.0, 50 / 255.0));
    SetUniformMat4(shader_id, "modelMatrix", car);
    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
    
    mat4 wheel2 = translate(mat4(1.0f), sizeInc*vec3(4.5f + carMove.x, 0.5f, -3.0f + carMove.z))* rotate(mat4(1.0f), radians(rotX), sizeInc*vec3(1, 0, 0))*scale(mat4(1.0f), sizeInc* vec3(0.5f, 1.0f, 1.0f));
    car = reposition * wheel2;
    SetUniformVec3(shader_id, "object_color", vec3(50 / 255.0, 50 / 255.0, 50 / 255.0));
    SetUniformMat4(shader_id, "modelMatrix", car);
    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
    
    mat4 wheel3 = translate(mat4(1.0f), sizeInc*vec3(0 + carMove.x, 0.5f, -7.0f + carMove.z))* rotate(mat4(1.0f), radians(rotX), sizeInc*vec3(1, 0, 0))*scale(mat4(1.0f), sizeInc* vec3(0.5f, 1.0f, 1.0f));
    car = reposition * wheel3;
    SetUniformVec3(shader_id, "object_color", vec3(50 / 255.0, 50 / 255.0, 50 / 255.0));
    SetUniformMat4(shader_id, "modelMatrix", car);
    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
    
    mat4 wheel4 = translate(mat4(1.0f), sizeInc*vec3(0 + carMove.x, 0.5f, -3.0f + carMove.z))* rotate(mat4(1.0f), radians(rotX), sizeInc*vec3(1, 0, 0))*scale(mat4(1.0f), sizeInc* vec3(0.5f, 1.0f, 1.0f));
    car = reposition * wheel4;
    SetUniformVec3(shader_id, "object_color", vec3(50 / 255.0, 50 / 255.0, 50 / 255.0));
    SetUniformMat4(shader_id, "modelMatrix", car);
    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
    
}


//Draws the tree
void drawTree(GLuint shader, float z, float x, float initial, int tree, GLuint woodText, GLuint leafText) {
    
    if (tree == 1) {
        mat4 scaleDown = scale(mat4(1.0f), vec3(0.75, 0.75, 0.75));
        
        //Trunk
        glBindTexture(GL_TEXTURE_2D, woodText);
        
        mat4 trunkMatrix = translate(mat4(1.0f), vec3(initial + x, 5.0f, 0.0f + z)) * scale(mat4(1.0f), vec3(3.0f, 20.0f, 3.0f));
        trunkMatrix = scaleDown * trunkMatrix;
        SetUniformMat4(shader, "modelMatrix", trunkMatrix);
        SetUniformVec3(shader, "object_color", vec3(0.267f, 0.129f, 0.004f)); // Brown
        glDrawArrays(GL_TRIANGLES, 0, 36); // 36 vertices, starting at index 0
        
        //Top leaves
        glBindTexture(GL_TEXTURE_2D, leafText);
        
        mat4 leavesMatrix = translate(mat4(1.0f), vec3(initial + x, 10.0f, 0.0f + z)) * scale(mat4(1.0f), vec3(12.0f, 2.0f, 12.0f));
        leavesMatrix = scaleDown * leavesMatrix;
        SetUniformMat4(shader, "modelMatrix", leavesMatrix);
        SetUniformVec3(shader, "object_color", vec3(0.0f, 1.0f, 0.0f)); // Green
        glDrawArrays(GL_TRIANGLES, 0, 36); // 36 vertices, starting at index 0
        
        leavesMatrix = translate(mat4(1.0f), vec3(initial + x, 12.0f, 0.0f + z)) * scale(mat4(1.0f), vec3(10.0f, 2.0f, 10.0f));
        leavesMatrix = scaleDown * leavesMatrix;
        SetUniformMat4(shader, "modelMatrix", leavesMatrix);
        SetUniformVec3(shader, "object_color", vec3(0.0f, 1.0f, 0.0f)); // Green
        glDrawArrays(GL_TRIANGLES, 0, 36); // 36 vertices, starting at index 0
        
        leavesMatrix = translate(mat4(1.0f), vec3(initial + x, 14.0f, 0.0f + z)) * scale(mat4(1.0f), vec3(8.0f, 2.0f, 8.0f));
        leavesMatrix = scaleDown * leavesMatrix;
        SetUniformMat4(shader, "modelMatrix", leavesMatrix);
        SetUniformVec3(shader, "object_color", vec3(0.0f, 1.0f, 0.0f)); // Green
        glDrawArrays(GL_TRIANGLES, 0, 36); // 36 vertices, starting at index 0
        
        leavesMatrix = translate(mat4(1.0f), vec3(initial + x, 16.0f, 0.0f + z)) * scale(mat4(1.0f), vec3(6.0f, 2.0f, 6.0f));
        leavesMatrix = scaleDown * leavesMatrix;
        SetUniformMat4(shader, "modelMatrix", leavesMatrix);
        SetUniformVec3(shader, "object_color", vec3(0.0f, 1.0f, 0.0f)); // Green
        glDrawArrays(GL_TRIANGLES, 0, 36); // 36 vertices, starting at index 0
        
        leavesMatrix = translate(mat4(1.0f), vec3(initial + x, 18.0f, 0.0f + z)) * scale(mat4(1.0f), vec3(4.0f, 2.0f, 4.0f));
        leavesMatrix = scaleDown * leavesMatrix;
        SetUniformMat4(shader, "modelMatrix", leavesMatrix);
        SetUniformVec3(shader, "object_color", vec3(0.0f, 1.0f, 0.0f)); // Green
        glDrawArrays(GL_TRIANGLES, 0, 36); // 36 vertices, starting at index 0
        
        leavesMatrix = translate(mat4(1.0f), vec3(initial + x, 20.0f, 0.0f + z)) * scale(mat4(1.0f), vec3(2.0f, 2.0f, 2.0f));
        leavesMatrix = scaleDown * leavesMatrix;
        SetUniformMat4(shader, "modelMatrix", leavesMatrix);
        SetUniformVec3(shader, "object_color", vec3(0.0f, 1.0f, 0.0f)); // Green
        glDrawArrays(GL_TRIANGLES, 0, 36); // 36 vertices, starting at index 0
        
        leavesMatrix = translate(mat4(1.0f), vec3(initial + x, 21.5f, 0.0f + z)) * scale(mat4(1.0f), vec3(1.0f, 1.0f, 1.0f));
        leavesMatrix = scaleDown * leavesMatrix;
        SetUniformMat4(shader, "modelMatrix", leavesMatrix);
        SetUniformVec3(shader, "object_color", vec3(0.0f, 1.0f, 0.0f)); // Green
        glDrawArrays(GL_TRIANGLES, 0, 36); // 36 vertices, starting at index 0
        
    }
    
    else if (tree == 2) {
        //Trunk
        mat4 groundWorldMatrix = translate(mat4(1.0f), vec3(initial + x, 3.0f, 0.0f + z)) * scale(mat4(1.0f), vec3(1.0f, 6.0f, 1.0f));
        glBindTexture(GL_TEXTURE_2D, woodText);
        SetUniformVec3(shader, "object_color", vec3(150.0 / 255.0, 75.0 / 255.0, 0.0f));
        SetUniformMat4(shader, "modelMatrix", groundWorldMatrix);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        //Leaves
        groundWorldMatrix = translate(mat4(1.0f), vec3(initial + x, 7.5f, 0.0f + z)) * scale(mat4(1.0f), vec3(4.0f, 3.0f, 4.0f));
        glBindTexture(GL_TEXTURE_2D, leafText);
        SetUniformVec3(shader, "object_color", vec3(0.0, 1.0, 0.0f));
        SetUniformMat4(shader, "modelMatrix", groundWorldMatrix);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    
}

//Function to draw bushes
void drawBushOrRock(GLuint shader, float z, float x, float initial, int draw, GLuint text) {
    if (draw == 1) {
        mat4 bushMatrix = translate(mat4(1.0f), vec3(initial + x, 1.0f, 0.0f + z)) * scale(mat4(1.0f), vec3(2.0f, 2.0f, 2.0f));
        glBindTexture(GL_TEXTURE_2D, text);
        SetUniformMat4(shader, "modelMatrix", bushMatrix);
        SetUniformVec3(shader, "object_color", vec3(0.0f, 1.0f, 0.5f)); // Green
        glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
    }
    
}


//Draws the rock
void drawRock(GLuint shader_id, float z, float x, float initial) {
    float sizeInc = 0.5;//make rock dif sizes
    mat4 reposition = translate(mat4(1.0f), vec3(0, 2, 0));//position rock in scene
    mat4 rock = reposition * translate(mat4(1.0f), sizeInc*(vec3(initial+x-5.0f, 4.0f, 3.0f+z ))) * scale(mat4(1.0f), sizeInc*vec3(0.5, 0.5f, 0.5f));
    SetUniformVec3(shader_id, "object_color", vec3(0,1,1));
    SetUniformMat4(shader_id, "modelMatrix", rock);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}



// Variables to control so that a button must be clicked and can't be held
int lastSpace = GLFW_RELEASE;
int previousXstate = GLFW_RELEASE;




int main(int argc, char* argv[])
{
    if (!InitContext()) return -1;
    
    // Load Textures
    GLuint bushTextureID = loadTexture((pathPrefix + "Assets/Textures/bush.jpg").c_str());
    GLuint leavesTextureID = loadTexture((pathPrefix + "Assets/Textures/leaves.png").c_str());
    GLuint woodTextureID = loadTexture((pathPrefix + "Assets/Textures/wood.jpg").c_str());
    GLuint roadTextureID = loadTexture((pathPrefix + "Assets/Textures/road.jpg").c_str());
    GLuint dirtTextureID = loadTexture((pathPrefix + "Assets/Textures/dirt.png").c_str());
    GLuint carTextureID = loadTexture((pathPrefix + "Assets/Textures/car.jpg").c_str());
    GLuint tireTextureID = loadTexture((pathPrefix + "Assets/Textures/tire.jpg").c_str());
    
    
    
    
    // Gray background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    
    
    // Compile and link shaders here ...
    int lightShaderProgram = compileAndLinkShaders(getLightVertexShaderSource(), getLightFragmentShaderSource());
    //int ShadowShaderProgram = compileAndLinkShaders(getShadowVertexShaderSource(), getShadowFragmentShaderSource());
    int textureFlag = 1; // Toggle textures flag
    
    SetUniformVec3(lightShaderProgram, "light_color", vec3(1.0, 1.0, 1.0)); // Set light color on light shader
    
    SetUniformVec3(lightShaderProgram, "light_color2", vec3(1.0, 1.0, 1.0)); // Set light color on light shader
    
    
    // Camera parameters for view transform
    vec3 cameraPosition(0.0f, 3.0f, 0.0f);
    vec3 cameraLookAt(0.0f, 0.0f, -1.0f);
    vec3 cameraUp(0.0f, 1.0f, 0.0f);
    
    vec3 carMove(0,0,0);
    
    // Other camera parameters
    float cameraSpeed = 1.0f;
    float cameraHorizontalAngle = 90.0f;
    float cameraVerticalAngle = 0.0f;
    
    // Define and upload geometry to the GPU here ...
    int texturedCubeVAO = createTexturedCubeVertexArrayObject();
    int lightCubeVAO = createLightCubeVertexArrayObject();
    int vaos = createSphereObject();
    
    // For frame time
    float lastFrameTime = glfwGetTime();
    int lastMouseLeftState = GLFW_RELEASE;
    double lastMousePosX, lastMousePosY;
    glfwGetCursorPos(window, &lastMousePosX, &lastMousePosY);
    
    
    // Other OpenGL states to set once
    //Enable Backface culling
    glEnable(GL_DEPTH_TEST);
    
    
    //Variables used for key input rotation
    float i = 0; //the angle of rotation (also serves as left/right shoulder rotation
    float scaler = 1;
    
    
    float lr = 0; //Float to move the model left and right
    float ud = 0; //Float to move the model up and down
    
    // Variable that determines whether to render as triangles (0), lines (1) or points (2)
    GLenum mode = GL_TRIANGLES;
    
    // Used for the world orientation
    float anglex = 0.0;
    float angley = 0.0;
    
    
    //For button presses
    int lastSpaceState = GLFW_RELEASE;
    
    //Value for zoom
    float fovScale = 0;
    float fov = 70;
    
    //Used for toggling textures
    int lastCState = GLFW_RELEASE;
    int shaderMode = 0;
    
    //For toggling lights
    int lastLState = GLFW_RELEASE;
    
    
    
    
    
    
    
    //Stuff for procedural generation
    float camLoc = 0.0f;
    
    
    // "Random" coordinates
    float coords[1000];
    for (int i = 0; i < 1000; i++) {
        coords[i] = (rand() % 5);
    }
    
    float coords2[1000];
    for (int i = 0; i < 1000; i++) {
        coords2[i] = (rand() % 5);
    }
    
    float coords3[1000];
    for (int i = 0; i < 1000; i++) {
        coords3[i] = (rand() % 5);
    }
    
    int coordsLoc = 0;
    float dist = 350;
    
    
    int tree[1000];
    for (int i = 0; i < 1000; i++) {
        tree[i] = ((rand() % 2) + 1);
    }
    
    int treeLoc = 0;
    
    
    float bush[1000];
    for (int i = 0; i < 1000; i++) {
        bush[i] = ((rand() % 3) + 1);
    }
    
    float bushMove[1000];
    for (int i = 0; i < 1000; i++) {
        bushMove[i] = (rand() % 8);
    }
    
    int bushLoc = 0;
    
    
    float rock[1000];
    for (int i = 0; i < 1000; i++) {
        rock[i] = ((rand() % 4) + 1);
    }
    float rockMove[1000];
    for (int i = 0; i < 1000; i++) {
        rockMove[i] = ((rand() % 4) + 1);
    }
    int rockLoc = 0;
    
    
    // Entering Main Loop
    while (!glfwWindowShouldClose(window))
    {
        //Keeps track of keys
        lastCState = glfwGetKey(window, GLFW_KEY_C);
        lastSpaceState = glfwGetKey(window, GLFW_KEY_SPACE);
        lastLState = glfwGetKey(window, GLFW_KEY_L);
        
        
        // Frame time calculation
        float dt = glfwGetTime() - lastFrameTime;
        lastFrameTime += dt;
        
        // Each frame, reset color of each pixel to glClearColor
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Set projection matrix
        mat4 projectionMatrix = perspective(fov, // field of view in degrees
                                            1024.0f / 768.0f,  // aspect ratio
                                            0.01f, 200.0f);   // near and far (near > 0)
        
        // Set Projection matrix on all shaders
        SetUniformMat4(lightShaderProgram, "projectionMatrix", projectionMatrix); // Set projection matrix on light shader
        
        
        if (camNum == 1) {cameraPosition= vec3(0.0f+carMove.x, 3.0f, 0.0f + carMove.z);}
        else if (camNum == 2) { cameraPosition = vec3(0.0f + carMove.x, 3.25f, 5.25f + carMove.z); }
        else if (camNum == 3) { cameraPosition = vec3(0.0f + carMove.x, 8.0f, 20.0f + carMove.z); }
        else if (camNum == 4) { cameraPosition = vec3(0.0f + carMove.x, 30.0f, 20.0f + carMove.z); }
        
        
        // Light parameters for point light (light one)
        vec3 lightPosition = vec3(0.0f, 20.0f, 0.0f + camLoc); // the location of the light in 3D space
        vec3 lightFocus(0.0, -1.0, 0.0);      // the point in 3D space the light "looks" at
        vec3 lightDirection = normalize(lightFocus - lightPosition);
        
        float lightNearPlane = 1.0f; //1
        float lightFarPlane = 180.0f; //180
        
        mat4 lightProjectionMatrix = frustum(-1.0f, 1.0f, -1.0f, 1.0f, lightNearPlane, lightFarPlane);
        mat4 lightViewMatrix = lookAt(lightPosition, lightFocus, vec3(0.0f, 1.0f, 0.0f));
        mat4 lightSpaceMatrix = lightProjectionMatrix * lightViewMatrix;
        
        // Set light space matrix on both shaders
        SetUniformMat4(lightShaderProgram, "lightMatrix", lightSpaceMatrix);
        //SetUniformMat4(lightShaderProgram, "lightMatrix", lightSpaceMatrix);
        
        // Set light far and near planes on scene shader
        SetUniform1Value(lightShaderProgram, "light_near_plane", lightNearPlane);
        SetUniform1Value(lightShaderProgram, "light_far_plane", lightFarPlane);
        
        SetUniformVec3(lightShaderProgram, "light_position", lightPosition); // Set light position on scene shader
        SetUniformVec3(lightShaderProgram, "light_direction", lightDirection); // Set light direction on scene shader
        
        
        
        // Light parameters for point light (light two)
        vec3 lightPosition2 = vec3(1 + carMove.x, -0, 0.0f + carMove.z); // the location of the light in 3D space
        vec3 lightFocus2(0.0, -1.0, 10.0 + carMove.z);      // the point in 3D space the light "looks" at
        vec3 lightDirection2 = normalize(lightFocus2 - lightPosition2);
        
        float lightNearPlane2 = 1.0f; //1
        float lightFarPlane2 = 2.0f; //180
        
        mat4 lightProjectionMatrix2 = frustum(-1.0f, 1.0f, -1.0f, 1.0f, lightNearPlane2, lightFarPlane2);
        mat4 lightViewMatrix2 = lookAt(lightPosition2, lightFocus2, vec3(0.0f, 1.0f, 0.0f));
        mat4 lightSpaceMatrix2 = lightProjectionMatrix2 * lightViewMatrix2;
        
        // Set light space matrix on both shaders
        SetUniformMat4(lightShaderProgram, "lightMatrix2", lightSpaceMatrix2);
        //SetUniformMat4(lightShaderProgram, "lightMatrix", lightSpaceMatrix);
        
        // Set light far and near planes on scene shader
        SetUniform1Value(lightShaderProgram, "light_near_plane2", lightNearPlane2);
        SetUniform1Value(lightShaderProgram, "light_far_plane2", lightFarPlane2);
        
        SetUniformVec3(lightShaderProgram, "light_position2", lightPosition2); // Set light position on scene shader
        SetUniformVec3(lightShaderProgram, "light_direction2", lightDirection2); // Set light direction on scene shader
        
        //Angles for rotating spotlight
        float lightAngleOuter2 = 2.0;
        float lightAngleInner2 = 1.0;
        // Set light cutoff angles on scene shader
        SetUniformfvalue(lightShaderProgram, "light_cutoff_inner2", cos(radians(lightAngleInner2)));
        SetUniformfvalue(lightShaderProgram, "light_cutoff_outer2", cos(radians(lightAngleOuter2)));
        
        
        
        
        
        
        // Controls world orientation
        mat4 currentWorldOrientationMatrix = rotate(mat4(1.0f), radians(angley), vec3(1.0f, 0.0f, 0.0f)) * rotate(mat4(1.0f), radians(anglex), vec3(0.0f, 1.0f, 0.0f));
        SetUniformMat4(lightShaderProgram, "worldOrientationMatrix", currentWorldOrientationMatrix); // Set WOM matrix on light shader
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        SetUniform1Value(lightShaderProgram, "useTexture", textureFlag); // Set texture flag
        SetUniform1Value(lightShaderProgram, "useCarLight", carLight); // Set texture flag
        
        
        // Draw textured geometry
        glActiveTexture(GL_TEXTURE0);
        GLuint textureLocation = glGetUniformLocation(lightShaderProgram, "textureSampler");
        glUniform1i(textureLocation, 0);                // Set our Texture sampler to user Texture Unit 0
        
        
        
        //Bind sphere vertex
        glBindVertexArray(vaos);
        
        
        //Drawing initial bushes
        for (int i = -350; i < 350; i++) {
            for (float j = 10; j <= 85; j += 25) {
                if ((i % 10) == 0) {
                    drawBushOrRock(lightShaderProgram, i + 2, bushMove[bushLoc], j, bush[bushLoc], bushTextureID);
                }
                
                if (bushLoc < 999) {
                    bushLoc += 1;
                }
                
                else {
                    bushLoc = 0;
                }
                
            }
            
        }
        
        
        for (int i = -350; i < 350; i++) {
            for (float j = 15; j <= 90; j += 25) {
                if ((i % 10) == 0) {
                    drawBushOrRock(lightShaderProgram, i + 2, bushMove[bushLoc], -j, bush[bushLoc], bushTextureID);
                }
                
                if (bushLoc < 999) {
                    bushLoc += 1;
                }
                
                else {
                    bushLoc = 0;
                }
                
            }
            
        }
        
        
        bushLoc = 0;
        
        
        
        //Bind cube vertex
        glBindVertexArray(texturedCubeVAO);
        SetUniform1Value(lightShaderProgram, "useTexture", 0);
        
        
        
        
        
        //Draw the initial ground
        for (int i = -350; i <= 350; i++) {
            if ((i % 50) == 0) {
                drawGround(lightShaderProgram, i, dirtTextureID);
                drawRoad(lightShaderProgram, i, roadTextureID);
            }
        }
        
        
        SetUniform1Value(lightShaderProgram, "useTexture", textureFlag);
        //Drawing the initial trees (right side)
        for (int i = 350; i > -350; i--) {
            for (float j = 20.0; j <= 95.0; j += 25.0) {
                if ((i % 20) == 0) {
                    int val = coords[coordsLoc];
                    drawTree(lightShaderProgram, i, val, j, tree[treeLoc], woodTextureID, leavesTextureID);
                    
                    if (coordsLoc < 999) {
                        coordsLoc += 1;
                        treeLoc += 1;
                    }
                    
                    else {
                        coordsLoc = 0;
                        treeLoc = 0;
                    }
                }
            }
        }
        
        //Drawing the initial trees (left side)
        for (int i = 350; i > -350; i--) {
            for (float j = 25.0; j <= 100.0; j += 25.0) {
                if ((i % 20) == 0) {
                    int val = coords[coordsLoc];
                    drawTree(lightShaderProgram, i, val, -j, tree[treeLoc], woodTextureID, leavesTextureID);
                    
                    if (coordsLoc < 999) {
                        coordsLoc += 1;
                        treeLoc += 1;
                    }
                    
                    else {
                        coordsLoc = 0;
                        treeLoc = 0;
                    }
                }
            }
        }
        
        coordsLoc = 0;
        treeLoc = 0;
        
        
        
        
        
        
        
        
        //Generating the world when going forward (negative z direction)
        if (camLoc <= -50) {
            //Enabling textures
            SetUniform1Value(lightShaderProgram, "useTexture", textureFlag);
            //Bind sphere vertex
            glBindVertexArray(vaos);
            
            
            //Drawing bushes (right)
            for (int i = -350; i >= camLoc - 450; i--) {
                for (float j = 10; j <= 85; j += 25) {
                    if ((i % 10) == 0) {
                        drawBushOrRock(lightShaderProgram, i + 2, bushMove[bushLoc], j, bush[bushLoc], bushTextureID);
                    }
                    
                    if (bushLoc < 999) {
                        bushLoc += 1;
                    }
                    
                    else {
                        bushLoc = 0;
                    }
                    
                }
                
            }
            
            bushLoc = 0;
            
            //Drawing bushes (left)
            for (int i = -350; i >= camLoc - 450; i--) {
                for (float j = 15; j <= 90; j += 25) {
                    if ((i % 10) == 0) {
                        drawBushOrRock(lightShaderProgram, i + 2, bushMove[bushLoc], -j, bush[bushLoc], bushTextureID);
                    }
                    
                    if (bushLoc < 999) {
                        bushLoc += 1;
                    }
                    
                    else {
                        bushLoc = 0;
                    }
                    
                }
                
            }
            
            
            bushLoc = 0;
            
            
            //Bind cube vertex
            glBindVertexArray(texturedCubeVAO);
            SetUniform1Value(lightShaderProgram, "useTexture", 0);
            
            
            //Calling the drawGround and drawRoad function
            for (int i = -350; i >= camLoc - 200; i--) {
                if ((i % 50) == 0) {
                    drawGround(lightShaderProgram, i, dirtTextureID);
                    drawRoad(lightShaderProgram, i, roadTextureID);
                }
            }
            
            
            
            SetUniform1Value(lightShaderProgram, "useTexture", textureFlag);
            //Calling the drawTree function (right side)
            for (int i = -350; i >= camLoc - 450; i--) {
                for (float j = 20.0; j <= 95.0; j += 25.0) {
                    if ((i % 20) == 0) {
                        int val = coords[coordsLoc];
                        drawTree(lightShaderProgram, i, val, j, tree[treeLoc], woodTextureID, leavesTextureID);
                        
                        if (coordsLoc < 999) {
                            coordsLoc += 1;
                            treeLoc += 1;
                        }
                        
                        else {
                            coordsLoc = 0;
                            treeLoc = 0;
                        }
                    }
                }
                
                
            }
            
            coordsLoc = 0;
            treeLoc = 0;
            
            
            //Calling the drawTree function (left side)
            for (int i = -350; i >= camLoc - 450; i--) {
                for (float j = 25.0; j <= 100.0; j += 25.0) {
                    if ((i % 20) == 0) {
                        int val = coords[coordsLoc];
                        drawTree(lightShaderProgram, i, val, -j, tree[treeLoc], woodTextureID, leavesTextureID);
                        
                        if (coordsLoc < 999) {
                            coordsLoc += 1;
                            treeLoc += 1;
                        }
                        
                        else {
                            coordsLoc = 0;
                            treeLoc = 0;
                        }
                    }
                }
                
                
            }
            
            
            coordsLoc = 0;
            treeLoc = 0;
            
            
        }//End of if statement
            
            
            
            
            
            
            //Generating the world going backwards (positive z direction)
        else if (camLoc >= 50) {
            //Enabling textures
            SetUniform1Value(lightShaderProgram, "useTexture", textureFlag);
            //Bind sphere vertex
            glBindVertexArray(vaos);
            
            
            //Drawing bushes (right)
            for (int i = 350; i <= camLoc + 450; i++) {
                for (float j = 10; j <= 85; j += 25) {
                    if ((i % 10) == 0) {
                        drawBushOrRock(lightShaderProgram, i + 2, bushMove[bushLoc], j, bush[bushLoc], bushTextureID);
                    }
                    
                    if (bushLoc < 999) {
                        bushLoc += 1;
                    }
                    
                    else {
                        bushLoc = 0;
                    }
                    
                }
                
            }
            
            bushLoc = 0;
            
            //Drawing bushes (left)
            for (int i = 350; i <= camLoc + 450; i++) {
                for (float j = 15; j <= 90; j += 25) {
                    if ((i % 10) == 0) {
                        drawBushOrRock(lightShaderProgram, i + 2, bushMove[bushLoc], -j, bush[bushLoc], bushTextureID);
                    }
                    
                    if (bushLoc < 999) {
                        bushLoc += 1;
                    }
                    
                    else {
                        bushLoc = 0;
                    }
                    
                }
                
            }
            
            
            bushLoc = 0;
            
            
            
            
            //Bind cube vertex
            glBindVertexArray(texturedCubeVAO);
            SetUniform1Value(lightShaderProgram, "useTexture", 0);
            
            
            
            //Calling the drawGround and drawRoad function
            for (int i = 350; i <= camLoc + 200.0; i++) {
                if ((i % 50) == 0) {
                    drawGround(lightShaderProgram, i, dirtTextureID);
                    drawRoad(lightShaderProgram, i, roadTextureID);
                }
            }
            
            
            //Calling the drawTree function (left side)
            for (int i = 350; i <= camLoc + 450; i++) {
                for (float j = 20.0; j <= 95.0; j += 25.0) {
                    if ((i % 20) == 0) {
                        int val = coords[coordsLoc];
                        drawTree(lightShaderProgram, i, val, j, tree[treeLoc], woodTextureID, leavesTextureID);
                        
                        if (coordsLoc < 999) {
                            coordsLoc += 1;
                            treeLoc += 1;
                        }
                        
                        else {
                            coordsLoc = 0;
                            treeLoc = 0;
                        }
                    }
                }
                
                
            }
            
            coordsLoc = 0;
            treeLoc = 0;
            
            
            
            SetUniform1Value(lightShaderProgram, "useTexture", textureFlag);
            //Calling the drawTree function (right side)
            for (int i = 350; i <= camLoc + 450; i++) {
                for (float j = 25.0; j <= 100.0; j += 25.0) {
                    if ((i % 20) == 0) {
                        int val = coords[coordsLoc];
                        drawTree(lightShaderProgram, i, val, -j, tree[treeLoc], woodTextureID, leavesTextureID);
                        
                        if (coordsLoc < 999) {
                            coordsLoc += 1;
                            treeLoc += 1;
                        }
                        
                        else {
                            coordsLoc = 0;
                            treeLoc = 0;
                        }
                    }
                }
                
                
            }
            
            SetUniform1Value(lightShaderProgram, "useTexture", textureFlag);
            //Calling the drawTree function (right side)
            for (int i = 350; i <= camLoc + 400; i++) {
                for (float j = 25.0; j <= 100.0; j += 30.0) {
                    if ((i % 20) == 0) {
                        int val = coords[coordsLoc];
                        drawRock(lightShaderProgram, i, val, -j);
                        
                        if (coordsLoc < 999) {
                            coordsLoc += 1;
                            rockLoc += 1;
                        }
                        
                        else {
                            coordsLoc = 0;
                            rockLoc = 0;
                        }
                    }
                }
                
                
            }
            
            
            
        }//End of else statement
        
        
        
        
        
        
        drawCar(lightShaderProgram, vaos, carMove, carTextureID, tireTextureID);
        
        
        
        
        
        glUseProgram(lightShaderProgram);
        mat4 viewMatrix = lookAt(cameraPosition, cameraPosition + cameraLookAt, cameraUp); // Set the view matrix for first person camera and send to both shaders
        SetUniformMat4(lightShaderProgram, "viewMatrix", viewMatrix);
        SetUniformVec3(lightShaderProgram, "view_position", cameraPosition); // Set view position on scene shader
        SetUniformMat4(lightShaderProgram, "projectionMatrix", projectionMatrix); // Set projection matrix on both shaders
        
        
        
        
        
        
        
        
        
        // End Frame
        glfwSwapBuffers(window);
        glfwPollEvents();
        
        
        // Handle inputs
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        
        
        // Calculate mouse motion dx and dy
        // Update camera horizontal and vertical angle
        double mousePosX, mousePosY;
        glfwGetCursorPos(window, &mousePosX, &mousePosY);
        
        double dx = mousePosX - lastMousePosX;
        double dy = mousePosY - lastMousePosY;
        
        lastMousePosX = mousePosX;
        lastMousePosY = mousePosY;
        
        
        // Convert to spherical coordinates
        const float cameraAngularSpeed = 20.0f;
        
        //Allows free movement of the mouse
        cameraHorizontalAngle -= dx * cameraAngularSpeed * dt;
        cameraVerticalAngle -= dy * cameraAngularSpeed * dt;
        
        
        // Clamp vertical angle to [-85, 85] degrees
        cameraVerticalAngle = std::max(-85.0f, std::min(85.0f, cameraVerticalAngle));
        if (cameraHorizontalAngle > 360)
        {
            cameraHorizontalAngle -= 360;
        }
        else if (cameraHorizontalAngle < -360)
        {
            cameraHorizontalAngle += 360;
        }
        
        float theta = radians(cameraHorizontalAngle);
        float phi = radians(cameraVerticalAngle);
        
        cameraLookAt = vec3(cosf(phi) * cosf(theta), sinf(phi), -cosf(phi) * sinf(theta));
        vec3 cameraSideVector = glm::cross(cameraLookAt, vec3(0.0f, 1.0f, 0.0f));
        
        glm::normalize(cameraSideVector);
        
        
        
        
        
        
        
        if (lastLState == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) // turn car lights on and off
        {
            if (carLight == 0) { carLight = 1; }
            else { carLight = 0; }
        }
        
        //Move the world orientation left
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) // move world to the left
        {
            anglex += 0.05f;
        }
        
        //Move the world orientation right
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) // move world to the right
        {
            anglex -= 0.05;
        }
        
        //Move the world orientation down
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) // move world up
        {
            angley -= 0.05f;
        }
        
        //Move the world orientation up
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) // move world down
        {
            angley += 0.05f;
        }
        
        
        //Move the camera left
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) // move camera to the left
        {
            cameraPosition.x -= 0.05f;
            carMove.x -= 0.05f;
            if (cameraPosition.x < -4.0f) {
                cameraPosition.x = -4.0f;
            }
            if (carMove.x < -3.0f) {
                carMove.x = -3.0f;
            }
            
        }
        
        //Move the camera to the right
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) // move camera to the right
        {
            cameraPosition.x += 0.05f;
            carMove.x += 0.05f;
            if (cameraPosition.x > 4.0f) {
                cameraPosition.x = 4.0f;
            }
            if (carMove.x > 3.0f) {
                carMove.x = 3.0f;
            }
        }
        
        //Move the camera backwards
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) // move camera up
        {
            cameraPosition.z -= cameraLookAt.z * 0.2;
            cameraPosition.x -= cameraLookAt.x * 0.2;
            
            carMove.x -= cameraLookAt.x * 0.2;
            carMove.z -= cameraLookAt.z * 0.2;
            
            if (cameraPosition.x < -4.0f) {
                cameraPosition.x = -4.0f;
            }
            if (cameraPosition.x > 4.0f) {
                cameraPosition.x = 4.0f;
            }
            
            if (carMove.x < -3.0f) {
                carMove.x = -3.0f;
            }
            if (carMove.x > 3.0f) {
                carMove.x = 3.0f;
            }
            
            camLoc -= cameraLookAt.z * 0.2;
            dist + -cameraLookAt.z * 0.2;
            
        }
        
        //Move the camera forwards
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) // move camera down
        {
            cameraPosition.z += cameraLookAt.z * 0.2;
            cameraPosition.x += cameraLookAt.x * 0.2;
            
            carMove.x += cameraLookAt.x * 0.2;
            carMove.z += cameraLookAt.z * 0.2;
            
            if (cameraPosition.x < -4.0f) {
                cameraPosition.x = -4.0f;
            }
            if (cameraPosition.x > 4.0f) {
                cameraPosition.x = 4.0f;
            }
            
            if (carMove.x < -3.0f) {
                carMove.x = -3.0f;
            }
            if (carMove.x > 3.0f) {
                carMove.x = 3.0f;
            }
            
            camLoc += cameraLookAt.z * 0.2;
            dist += cameraLookAt.z * 0.2;
            
            
        }
        
        
        
        //Allow the user to zoom in and out
        if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
            const float cameraAngularSpeed = 1.0f;
            fovScale -= dy * cameraAngularSpeed * dt;
            fov = fov - fovScale;
            fovScale = 0;
            
            if (fov < 69.5f) {
                fov = 69.5f;
            }
            
            if (fov > 71.0f) {
                fov = 71.0f;
            }
            
            
        }
        
        
        //Change between aerial and person view
        if (lastCState == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
            
            if (camNum == 1) {
                camNum = 2;
            }
            else if (camNum == 2) {
                camNum = 3;
            }
            else if (camNum == 3) {
                camNum = 4;
            }
            else if (camNum == 4) {
                camNum = 1;
            }
        }
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        viewMatrix = lookAt(cameraPosition, cameraPosition + cameraLookAt, cameraUp);
        SetUniformMat4(lightShaderProgram, "viewMatrix", viewMatrix);
        SetUniformVec3(lightShaderProgram, "view_position", cameraPosition); // Set view position on light shader
        
    }
    
    // Shutdown GLFW
    glfwTerminate();
    
    return 0;
}




const char* getLightVertexShaderSource()
{
    return
            "#version 330 core\n"
            ""
            "uniform vec3 view_position;"
            ""
            "layout(location = 0) in vec3 position;"
            "layout(location = 1) in vec3 normals;"
            "layout (location = 2) in vec2 aUV;"
            ""
            "uniform mat4 modelMatrix;"
            "uniform mat4 viewMatrix;"
            "uniform mat4 projectionMatrix;"
            "uniform mat4 worldOrientationMatrix;"
            "uniform mat4 lightMatrix;"
            "uniform mat4 lightMatrix2;"
            ""
            "out vec3 fragment_normal;"
            "out vec3 fragment_position;"
            "out vec4 fragment_position_light_space;"
            "out vec4 fragment_position_light_space2;"
            "out vec2 vertexUV;"
            ""
            "void main()"
            "{"
            "fragment_normal = mat3(modelMatrix) * normals;"
            "fragment_position = vec3(modelMatrix * vec4(position, 1.0));"
            "fragment_position_light_space = lightMatrix * vec4(fragment_position, 1.0);"
            "fragment_position_light_space2 = lightMatrix2 * vec4(fragment_position, 1.0);"
            "gl_Position = projectionMatrix * viewMatrix * worldOrientationMatrix * modelMatrix * vec4(position, 1.0);"
            "   vertexUV = aUV;"
            "}";
}

const char* getLightFragmentShaderSource()
{
    return
            "#version 330 core\n"
            ""
            "uniform vec3 light_color;"
            "uniform vec3 light_position;"
            "uniform vec3 light_direction;"
            ""
            "uniform vec3 light_color2;"
            "uniform vec3 light_position2;"
            "uniform vec3 light_direction2;"
            ""
            "uniform vec3 object_color;"
            ""
            "const float shading_ambient_strength = 0.7;"
            "const float shading_diffuse_strength = 0.6;"
            "const float shading_specular_strength = 0.3;"
            ""
            "const float shading_ambient_strength2 = 0.7;"
            "const float shading_diffuse_strength2 = 0.6;"
            "const float shading_specular_strength2 = 0.3;"
            ""
            "uniform float light_cutoff_inner;"
            "uniform float light_cutoff_outer;"
            "uniform float light_near_plane;"
            "uniform float light_far_plane;"
            ""
            "uniform float light_cutoff_inner2;"
            "uniform float light_cutoff_outer2;"
            "uniform float light_near_plane2;"
            "uniform float light_far_plane2;"
            ""
            "uniform vec3 view_position;"
            "uniform sampler2D shadow_map;"
            "uniform sampler2D textureSampler;"
            "uniform int useTexture;"
            "uniform int useCarLight;"
            ""
            "in vec3 fragment_position;"
            "in vec4 fragment_position_light_space;"
            "in vec4 fragment_position_light_space2;"
            "in vec3 fragment_normal;"
            "in vec2 vertexUV;"
            ""
            "in vec4 gl_FragCoord;"
            "out vec4 result;"
            ""
            "vec3 ambient_color(vec3 light_color_arg, float ambStrength) {"
            "return ambStrength * light_color_arg;"
            "}"
            ""
            "vec3 diffuse_color(vec3 light_color_arg, vec3 light_position_arg, float diffStrength) {"
            "vec3 light_direction = normalize(light_position_arg - fragment_position);"
            "return diffStrength * light_color_arg * max(dot(normalize(fragment_normal), light_direction), 0.0f);"
            "}"
            ""
            "vec3 specular_color(vec3 light_color_arg, vec3 light_position_arg, float specStrength) {"
            "vec3 light_direction = normalize(light_position_arg - fragment_position);"
            "vec3 view_direction = normalize(view_position - fragment_position);"
            "vec3 reflect_light_direction = reflect(-light_direction, normalize(fragment_normal));"
            "return specStrength * light_color_arg * pow(max(dot(reflect_light_direction, view_direction), 0.0f), 32);"
            "}"
            ""
            /*      "float shadow_scalar() {"
                   ""// this function returns 1.0 when the surface receives light, and 0.0 when it is in a shadow
                   ""// perform perspective divide
                   "vec3 normalized_device_coordinates = fragment_position_light_space.xyz / fragment_position_light_space.w;"
                   ""// transform to [0,1] range
                   "normalized_device_coordinates = normalized_device_coordinates * 0.5 + 0.5;"
                   ""// get closest depth value from light's perspective (using [0,1] range fragment_position_light_space as coords)
                   "float closest_depth = texture(shadow_map, normalized_device_coordinates.xy).r;"
                   ""// get depth of current fragment from light's perspective
                   "float current_depth = normalized_device_coordinates.z;"
                   ""// check whether current frag pos is in shadow
                   "float bias = 0;"  // bias applied in depth map: see shadow_vertex.glsl
                   "return ((current_depth - bias) < closest_depth) ? 1.0 : 0.0;"
                   "}"*/
            ""
            ""
            "float spotlight_scalar() {"
            "float theta = dot(normalize(fragment_position - light_position2), light_direction2);"
            "if (theta > light_cutoff_inner2)"
            "{return 0.0;}"
            "else if (theta > light_cutoff_outer2) {"
            "return (1.0 - cos(3.14* (theta - light_cutoff_outer2) / (light_cutoff_inner2 - light_cutoff_outer2))) / 2.0;"
            "}"
            "else {"
            "return 1.0;"
            "}"
            "}"
            "void main()"
            "{"
            "vec3 ambient = vec3(0.0f);"
            "vec3 diffuse = vec3(0.0f);"
            "vec3 specular = vec3(0.0f);"
            ""
            "vec3 diffuse2 = vec3(0.0f);"
            "vec3 specular2 = vec3(0.0f);"
            ""
            "vec4 textureColor = texture(textureSampler, vertexUV);"
            ""
            //   "float scalar = shadow_scalar();"
            "ambient = 0.5*ambient_color(light_color, shading_ambient_strength);"
            "diffuse = 0.1*diffuse_color(light_color, light_position, shading_diffuse_strength);"
            "specular =  0.1*specular_color(light_color, light_position, shading_specular_strength);"
            ""
            "if (useCarLight==1){"
            "diffuse2 =  spotlight_scalar()*diffuse_color(light_color2, light_position2, shading_diffuse_strength2);"
            "specular2 =  spotlight_scalar()*specular_color(light_color2, light_position2, shading_specular_strength2);"
            "}"
            "else{}"
            "vec3 color = ((specular+specular2 + diffuse+diffuse2 + ambient)) * object_color;"
            ""
            "   if (useTexture == 0){"
            "  result =  vec4(color, 1.0f); }"
            "   else{"
            "  result = textureColor * vec4(color, 1.0f); }"
            "}";
}

/*
const char* getShadowVertexShaderSource()
{
return
	"#version 330 core\n"
	"layout(location = 0) in vec3 position;"
	"layout (location = 2) in vec2 aUV"
	""
	"uniform mat4 lightMatrix;"
	"uniform mat4 modelMatrix;"
	""
	"void main()"
	"{"
	"mat4 scale_bias_matrix = mat4(vec4(0.5, 0.0, 0.0, 0.0),"
	"vec4(0.0, 0.5, 0.0, 0.0),"
	"vec4(0.0, 0.0, 0.5, 0.0),"
	"vec4(0.5, 0.5, 0.5, 1.0));"
	"gl_Position ="
	""       //                    scale_bias_matrix * // bias the depth map coordinates
	"lightMatrix * modelMatrix * vec4(position, 1.0);"
	"}";
}

const char* getShadowFragmentShaderSource()
{
return
	"#version 330 core\n"
	""
	"out vec4 FragColor;"
	""
	"in vec4 gl_FragCoord;"
	""
	"void main()"
	"{"
		"gl_FragDepth = gl_FragCoord.z;"
		"FragColor = vec4(vec3(gl_FragCoord.z), 1.0f);"
	"}";
}
*/






int compileAndLinkShaders(const char* vertexShaderSource, const char* fragmentShaderSource)
{
    // compile and link shader program
    // return shader program id
    // ------------------------------------
    
    // vertex shader
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    // fragment shader
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    // link shaders
    int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return shaderProgram;
}

GLuint loadTexture(const char* filename)
{
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
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (!data)
    {
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

int createTexturedCubeVertexArrayObject()
{
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
                          (void*)0             // array buffer offset
    );
    glEnableVertexAttribArray(0);
    
    
    glVertexAttribPointer(1,                            // attribute 1 matches aColor in Vertex Shader
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(TexturedColoredVertex),
                          (void*)sizeof(vec3)      // color is offseted a vec3 (comes after position)
    );
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2,                            // attribute 2 matches aUV in Vertex Shader
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(TexturedColoredVertex),
                          (void*)(2 * sizeof(vec3))      // uv is offseted by 2 vec3 (comes after position and color)
    );
    glEnableVertexAttribArray(2);
    
    return vertexArrayObject;
}

int createLightCubeVertexArrayObject()
{
    // Create a vertex array
    GLuint lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);
    
    GLuint vertexBufferObject;
    glGenBuffers(1, &vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texturedCubeVertexArray), texturedCubeVertexArray, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0,                   // attribute 0 matches aPos in Vertex Shader
                          3,                   // size
                          GL_FLOAT,            // type
                          GL_FALSE,            // normalized?
                          sizeof(TexturedColoredVertex), // stride - each vertex contain 2 vec3 (position, color)
                          (void*)0             // array buffer offset
    );
    glEnableVertexAttribArray(0);
    
    
    glVertexAttribPointer(1,                            // attribute 1 matches aColor in Vertex Shader
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(TexturedColoredVertex),
                          (void*)sizeof(vec3)      // color is offseted a vec3 (comes after position)
    );
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2,                            // attribute 2 matches aUV in Vertex Shader
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(TexturedColoredVertex),
                          (void*)(2 * sizeof(vec3))      // uv is offseted by 2 vec3 (comes after position and color)
    );
    glEnableVertexAttribArray(2);
    
    return lightCubeVAO;
}

int createSphereObject()
{
    // A vertex is a point on a polygon, it contains positions and other data (eg: colors)
    unsigned int vaos;
    glGenVertexArrays(1, &vaos);
    
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
    for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
    {
        for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
        {
            float xSegment = (float)x / (float)X_SEGMENTS;
            float ySegment = (float)y / (float)Y_SEGMENTS;
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
    for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
    {
        if (!oddRow) // even rows: y == 0, y == 2; and so on
        {
            for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
            {
                indices.push_back(y * (X_SEGMENTS + 1) + x);
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
            }
        }
        else
        {
            for (int x = X_SEGMENTS; x >= 0; --x)
            {
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                indices.push_back(y * (X_SEGMENTS + 1) + x);
            }
        }
        oddRow = !oddRow;
    }
    indexCount = indices.size();
    
    std::vector<float> data;
    for (unsigned int i = 0; i < positions.size(); ++i)
    {
        data.push_back(positions[i].x);
        data.push_back(positions[i].y);
        data.push_back(positions[i].z);
        if (colors.size() > 0) {
            data.push_back(colors[i].x);
            data.push_back(colors[i].y);
            data.push_back(colors[i].z);
        }
        if (uv.size() > 0)
        {
            data.push_back(uv[i].x);
            data.push_back(uv[i].y);
        }
        if (normals.size() > 0)
        {
            data.push_back(normals[i].x);
            data.push_back(normals[i].y);
            data.push_back(normals[i].z);
        }
    }
    glBindVertexArray(vaos);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
    float stride = (3 + 2 + 3 + 3) * sizeof(float);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(8 * sizeof(float)));
    
    glBindBuffer(GL_ARRAY_BUFFER, 0); // VAO already stored the state we just defined, safe to unbind buffer
    glBindVertexArray(0); // Unbind to not modify the VAO
    
    return vaos;
}



bool InitContext()
{
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