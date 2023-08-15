
#define PATH_PREFIX "../" // Redefine your path prefix to the assets directory here

#define STB_IMAGE_IMPLEMENTATION

#include "shaders.h" // Note that GL is already included in shaders.h
#include <glm/glm.hpp>  // GLM is an optimized math library with syntax to similar to OpenGL Shading Language
#include <GLFW/glfw3.h> // GLFW provides a cross-platform interface for creating a graphical context,
#include <stb_image.h>

#include <algorithm>
#include <vector>
#include <map>
#include <random>
#include <cmath>

float rotX = 0.0f;
int camNum = 3;
bool carLight = false;

struct WorldChunk;
map<int, WorldChunk> chunksByPosition;

// Generates positions for items to be placed on the ground of a world chunk
struct PosGenerator {
    
    float startPositionZ; // The lower z-boundary of the world chunk which the item will be positioned on
    float x;              // Translation factor on x-axis
    float z;              // Translation factor on y-axis
    float itemSize;           // Scaling factor for the widest point of the item
    float gridSize = 100.0f; // How big the WorldChunk is (in 2D)
    float roadWidth = 6.0f;  // The width of the road + any additional offset where no objects should be
    bool leftSide;
    
    PosGenerator(float startPositionZ, float itemSize, bool leftSide = false) : startPositionZ(startPositionZ),
                                                                                itemSize(itemSize), leftSide(leftSide) {
        itemSize += 1.0f; // The 'size' is increased so that there is a bit of empty space between each tree
        
        float startPositionY = (itemSize / 2.0f) + (leftSide ? -gridSize / 2.0f : roadWidth / 2.0f);
        float endPositionY = -(itemSize / 2.0f) + (leftSide ? -roadWidth / 2.0f : gridSize / 2.0f);
        
        // Generate random positions within the world chunk boundaries
        random_device dev;
        default_random_engine generator(dev());
        
        uniform_real_distribution<float> xDistribution(startPositionY, endPositionY);
        uniform_real_distribution<float> zDistribution(startPositionZ + (itemSize / 2.0f),
                                                       startPositionZ + gridSize - (itemSize / 2.0f));
        
        x = xDistribution(generator);
        z = zDistribution(generator);
    };
    
};

struct WorldChunk {
    enum itemType {
        SMALL_TREE, BIG_TREE, BUSH, ROCK
    };
    
    vector<PosGenerator> bigTreePositions;
    vector<PosGenerator> smallTreePositions;
    vector<PosGenerator> rockPositions;
    vector<PosGenerator> bushPositions;
    
    // num of rows & cols = occupiable width/length of chunk + 1 for potential floating point errors
    bool occupiedGridsLeft[48][101] = {}; // Fill with false for all rows & cols
    bool occupiedGridsRight[48][101] = {};// Fill with false for all rows & cols
    float chunkPositionZ;
    int chunkPositionID;
    
    explicit WorldChunk(int chunkPositionID) : chunkPositionID(chunkPositionID) {
        chunkPositionZ = static_cast<float>((100 * chunkPositionID) + 50);
        
        generateItems(60);
    };
    
    bool insertItem(PosGenerator itemPos, itemType item) {
        vector<PosGenerator> *positions;
        
        switch (item) {
            case BUSH:
                positions = &bushPositions;
                break;
            case ROCK:
                positions = &rockPositions;
            case BIG_TREE:
                positions = &bigTreePositions;
                break;
            case SMALL_TREE:
                positions = &smallTreePositions;
                break;
            default:
                break;
        }
        
        bool (&occupiedGrids)[48][101] = itemPos.leftSide ? occupiedGridsLeft : occupiedGridsRight;
        
        // Convert item position to range [0, 47] and [0, 100]
        int x = static_cast<int>(itemPos.x + 50 - (itemPos.itemSize / 2)) + (itemPos.leftSide ? 0 : -53);
        int z = static_cast<int>(itemPos.z) - (100 * chunkPositionID + 50) - 2;
        
        // Check if the generated tree positions are already occupied
        for (int i = x; i < static_cast<int>(round(itemPos.itemSize)) + x; i++) {
            for (int j = z; j < static_cast<int>(round(itemPos.itemSize)) + z; j++) {
                if (occupiedGrids[i][j]) {
                    return false;
                }
            }
        }
        
        // Mark the new item's positions as occupied
        for (int i = x; i < static_cast<int>(round(itemPos.itemSize)) + x; i++) {
            for (int j = z; j < static_cast<int>(round(itemPos.itemSize)) + z; j++) {
                occupiedGrids[i][j] = true;
            }
        }
        
        positions->push_back(itemPos);
        
        return true;
    }
    
    void generateItems(int numOfItems) {
        while (numOfItems != 0) {
            // Big trees on right & left sides
            insertItem(PosGenerator(chunkPositionZ, 12.0f, false), BIG_TREE);
            insertItem(PosGenerator(chunkPositionZ, 12.0f, true), BIG_TREE);
            
            // Small trees on right & left sides
            insertItem(PosGenerator(chunkPositionZ, 5.0f, false), SMALL_TREE);
            insertItem(PosGenerator(chunkPositionZ, 5.0f, true), SMALL_TREE);
            
            // Bushes on right & left sides
            insertItem(PosGenerator(chunkPositionZ, 5.0f, false), BUSH);
            insertItem(PosGenerator(chunkPositionZ, 5.0f, true), BUSH);
            
            numOfItems -= 6;
        }
    }
    
    [[nodiscard]] mat4 getGroundMatrix() const {
        return translate(mat4(1.0f), vec3(0.0f, -0.3f, chunkPositionZ)) *
               scale(mat4(1.0f), vec3(100.0f, 0.1f, 100.0f));
    }
    
    [[nodiscard]] mat4 getRoadMatrix() const {
        return translate(mat4(1.0f), vec3(0.0f, -0.1f, chunkPositionZ)) *
               scale(mat4(1.0f), vec3(10.0f, 0.3f, 100.0f));
    }
};

unsigned int indexCount;

GLuint createTexturedCubeVAO();

GLuint createSphereObject();

GLuint loadTexture(const char *filename);

GLuint loadCubemap(vector<std::string> faces);

GLuint createSkyboxObject();

void renderScene(GLuint shader, GLuint texturedCubeVAO, GLuint sphereVAO, float cameraPosZ, GLuint roadTextureID,
                 GLuint dirtTextureID, GLuint woodTextureID, GLuint leavesTextureID,
                 GLuint carTextureID, GLuint tireTextureID, vec3 carMove);

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


//Function to draw bushes
void drawBush(GLuint shader, float z, float x, float initial, int draw, GLuint text) {
    if (draw == 1) {
        mat4 bushMatrix =
                translate(mat4(1.0f), vec3(initial + x, 1.0f, 0.0f + z)) * scale(mat4(1.0f), vec3(2.0f, 2.0f, 2.0f));
        glBindTexture(GL_TEXTURE_2D, text);
        SetUniformMat4(shader, "model_matrix", bushMatrix);
        SetUniformVec3(shader, "object_color", vec3(0.0f, 1.0f, 0.5f)); // Green
        glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
    }
    
}

//Draws the tree
void drawTree(GLuint shader, float z, float x, float initial, int tree, GLuint woodText, GLuint leafText) {
    
    if (tree == 1) {
        mat4 scaleDown = scale(mat4(1.0f), vec3(0.75f));
        mat4 translateXZ = translate(mat4(1.0f), vec3(x, 0.0f, z));
        
        //Trunk
        glBindTexture(GL_TEXTURE_2D, woodText);
        
        mat4 trunkMatrix =
                translate(mat4(1.0f), vec3(0.0f, 5.0f, 0.0f)) * scale(mat4(1.0f), vec3(3.0f, 20.0f, 3.0f));
        trunkMatrix = translateXZ * scaleDown * trunkMatrix;
        SetUniformMat4(shader, "model_matrix", trunkMatrix);
        SetUniformVec3(shader, "object_color", vec3(0.267f, 0.129f, 0.004f)); // Brown
        glDrawArrays(GL_TRIANGLES, 0, 36); // 36 vertices, starting at index 0
        
        //Top leaves
        glBindTexture(GL_TEXTURE_2D, leafText);
        
        mat4 leavesMatrix =
                translate(mat4(1.0f), vec3(0.0f, 10.0f, 0.0f)) * scale(mat4(1.0f), vec3(12.0f, 2.0f, 12.0f));
        leavesMatrix = translateXZ * scaleDown * leavesMatrix;
        SetUniformMat4(shader, "model_matrix", leavesMatrix);
        SetUniformVec3(shader, "object_color", vec3(0.0f, 1.0f, 0.0f)); // Green
        glDrawArrays(GL_TRIANGLES, 0, 36); // 36 vertices, starting at index 0
        
        leavesMatrix =
                translate(mat4(1.0f), vec3(0.0f, 12.0f, 0.0f)) * scale(mat4(1.0f), vec3(10.0f, 2.0f, 10.0f));
        leavesMatrix = translateXZ * scaleDown * leavesMatrix;
        SetUniformMat4(shader, "model_matrix", leavesMatrix);
        SetUniformVec3(shader, "object_color", vec3(0.0f, 1.0f, 0.0f)); // Green
        glDrawArrays(GL_TRIANGLES, 0, 36); // 36 vertices, starting at index 0
        
        leavesMatrix =
                translate(mat4(1.0f), vec3(0.0f, 14.0f, 0.0f)) * scale(mat4(1.0f), vec3(8.0f, 2.0f, 8.0f));
        leavesMatrix = translateXZ * scaleDown * leavesMatrix;
        SetUniformMat4(shader, "model_matrix", leavesMatrix);
        SetUniformVec3(shader, "object_color", vec3(0.0f, 1.0f, 0.0f)); // Green
        glDrawArrays(GL_TRIANGLES, 0, 36); // 36 vertices, starting at index 0
        
        leavesMatrix =
                translate(mat4(1.0f), vec3(0.0f, 16.0f, 0.0f)) * scale(mat4(1.0f), vec3(6.0f, 2.0f, 6.0f));
        leavesMatrix = translateXZ * scaleDown * leavesMatrix;
        SetUniformMat4(shader, "model_matrix", leavesMatrix);
        SetUniformVec3(shader, "object_color", vec3(0.0f, 1.0f, 0.0f)); // Green
        glDrawArrays(GL_TRIANGLES, 0, 36); // 36 vertices, starting at index 0
        
        leavesMatrix =
                translate(mat4(1.0f), vec3(0.0f, 18.0f, 0.0f)) * scale(mat4(1.0f), vec3(4.0f, 2.0f, 4.0f));
        leavesMatrix = translateXZ * scaleDown * leavesMatrix;
        SetUniformMat4(shader, "model_matrix", leavesMatrix);
        SetUniformVec3(shader, "object_color", vec3(0.0f, 1.0f, 0.0f)); // Green
        glDrawArrays(GL_TRIANGLES, 0, 36); // 36 vertices, starting at index 0
        
        leavesMatrix =
                translate(mat4(1.0f), vec3(0.0f, 20.0f, 0.0f)) * scale(mat4(1.0f), vec3(2.0f, 2.0f, 2.0f));
        leavesMatrix = translateXZ * scaleDown * leavesMatrix;
        SetUniformMat4(shader, "model_matrix", leavesMatrix);
        SetUniformVec3(shader, "object_color", vec3(0.0f, 1.0f, 0.0f)); // Green
        glDrawArrays(GL_TRIANGLES, 0, 36); // 36 vertices, starting at index 0
        
        leavesMatrix =
                translate(mat4(1.0f), vec3(0.0f, 21.5f, 0.0f)) * scale(mat4(1.0f), vec3(1.0f, 1.0f, 1.0f));
        leavesMatrix = translateXZ * scaleDown * leavesMatrix;
        SetUniformMat4(shader, "model_matrix", leavesMatrix);
        SetUniformVec3(shader, "object_color", vec3(0.0f, 1.0f, 0.0f)); // Green
        glDrawArrays(GL_TRIANGLES, 0, 36); // 36 vertices, starting at index 0
        
    } else if (tree == 2) {
        //Trunk
        mat4 groundWorldMatrix =
                translate(mat4(1.0f), vec3(x, 3.0f, z)) * scale(mat4(1.0f), vec3(1.0f, 6.0f, 1.0f));
        glBindTexture(GL_TEXTURE_2D, woodText);
        SetUniformVec3(shader, "object_color", vec3(150.0 / 255.0, 75.0 / 255.0, 0.0f));
        SetUniformMat4(shader, "model_matrix", groundWorldMatrix);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        //Leaves
        groundWorldMatrix =
                translate(mat4(1.0f), vec3(x, 7.5f, z)) * scale(mat4(1.0f), vec3(4.0f, 3.0f, 4.0f));
        glBindTexture(GL_TEXTURE_2D, leafText);
        SetUniformVec3(shader, "object_color", vec3(0.0, 1.0, 0.0f));
        SetUniformMat4(shader, "model_matrix", groundWorldMatrix);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

void drawCar(GLuint shader_id, int vaos, vec3 carMove, GLuint carText, GLuint tireText) {
    glBindTexture(GL_TEXTURE_2D, carText);
    mat4 car;
    float sizeInc = 1;//make car dif size
    mat4 reposition = translate(mat4(1.0f), vec3(-2.25, 0.5, 10.0f));//position car in scene
    
    mat4 body = translate(mat4(1.0f), sizeInc * vec3(2.25f + carMove.x, 1.0f, -5.0f + carMove.z)) *
                scale(mat4(1.0f), sizeInc * vec3(4.0f, 1.5f, 8.0f));
    car = reposition * body;
    SetUniformVec3(shader_id, "object_color", vec3(255 / 255.0, 105 / 255.0, 180 / 255.0));
    SetUniformMat4(shader_id, "model_matrix", car);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    mat4 light1 = translate(mat4(1.0f), sizeInc * vec3(1 + carMove.x, 1.0f, -9.0f + carMove.z)) *
                  scale(mat4(1.0f), sizeInc * vec3(0.5f, 0.5f, 0.1f));
    car = reposition * light1;
    SetUniformVec3(shader_id, "object_color", vec3(0, 1, 1));
    SetUniformMat4(shader_id, "model_matrix", car);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    mat4 light2 = translate(mat4(1.0f), sizeInc * vec3(3.5f + carMove.x, 1.0f, -9.0f + carMove.z)) *
                  scale(mat4(1.0f), sizeInc * vec3(0.5f, 0.5f, 0.1f));
    car = reposition * light2;
    SetUniformVec3(shader_id, "object_color", vec3(0, 1, 1));
    SetUniformMat4(shader_id, "model_matrix", car);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    mat4 side1 = translate(mat4(1.0f), sizeInc * vec3(0.75f + carMove.x, 2.5, -3 + carMove.z)) *
                 scale(mat4(1.0f), sizeInc * vec3(0.1f, 1.75, 3));
    car = reposition * side1;
    SetUniformVec3(shader_id, "object_color", vec3(255 / 255.0, 105 / 255.0, 180 / 255.0));
    SetUniformMat4(shader_id, "model_matrix", car);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    mat4 side1_2 = translate(mat4(1.0f), sizeInc * vec3(0.75f + carMove.x, 3.23, -6 + carMove.z)) *
                   scale(mat4(1.0f), sizeInc * vec3(0.1f, 0.3, 3));
    car = reposition * side1_2;
    SetUniformVec3(shader_id, "object_color", vec3(255 / 255.0, 105 / 255.0, 180 / 255.0));
    SetUniformMat4(shader_id, "model_matrix", car);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    mat4 side1_3 = translate(mat4(1.0f), sizeInc * vec3(0.75f + carMove.x, 1.75, -6 + carMove.z)) *
                   scale(mat4(1.0f), sizeInc * vec3(0.1f, 0.3, 3));
    car = reposition * side1_3;
    SetUniformVec3(shader_id, "object_color", vec3(255 / 255.0, 105 / 255.0, 180 / 255.0));
    SetUniformMat4(shader_id, "model_matrix", car);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    mat4 side1_4 = translate(mat4(1.0f), sizeInc * vec3(0.75f + carMove.x, 2.5, -7 + carMove.z)) *
                   scale(mat4(1.0f), sizeInc * vec3(0.15f, 1.3, 1));
    car = reposition * side1_4;
    SetUniformVec3(shader_id, "object_color", vec3(255 / 255.0, 105 / 255.0, 180 / 255.0));
    SetUniformMat4(shader_id, "model_matrix", car);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    mat4 side2 = translate(mat4(1.0f), sizeInc * vec3(3.75f + carMove.x, 2.5, -3 + carMove.z)) *
                 scale(mat4(1.0f), sizeInc * vec3(0.1f, 1.75, 3));
    car = reposition * side2;
    SetUniformVec3(shader_id, "object_color", vec3(255 / 255.0, 105 / 255.0, 180 / 255.0));
    SetUniformMat4(shader_id, "model_matrix", car);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    mat4 side2_2 = translate(mat4(1.0f), sizeInc * vec3(3.75f + carMove.x, 3.23, -6 + carMove.z)) *
                   scale(mat4(1.0f), sizeInc * vec3(0.1f, 0.3, 3));
    car = reposition * side2_2;
    SetUniformVec3(shader_id, "object_color", vec3(255 / 255.0, 105 / 255.0, 180 / 255.0));
    SetUniformMat4(shader_id, "model_matrix", car);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    mat4 side2_3 = translate(mat4(1.0f), sizeInc * vec3(3.75f + carMove.x, 1.75, -6 + carMove.z)) *
                   scale(mat4(1.0f), sizeInc * vec3(0.1f, 0.3, 3));
    car = reposition * side2_3;
    SetUniformVec3(shader_id, "object_color", vec3(255 / 255.0, 105 / 255.0, 180 / 255.0));
    SetUniformMat4(shader_id, "model_matrix", car);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    mat4 side2_4 = translate(mat4(1.0f), sizeInc * vec3(3.75f + carMove.x, 2.5, -7 + carMove.z)) *
                   scale(mat4(1.0f), sizeInc * vec3(0.15f, 1.3, 1));
    car = reposition * side2_4;
    SetUniformVec3(shader_id, "object_color", vec3(255 / 255.0, 105 / 255.0, 180 / 255.0));
    SetUniformMat4(shader_id, "model_matrix", car);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    mat4 window1 = translate(mat4(1.0f), sizeInc * vec3(2.25f + carMove.x, 1.75, -7.5 + carMove.z)) *
                   scale(mat4(1.0f), sizeInc * vec3(3, 0.3, 0.1f));
    car = reposition * window1;
    SetUniformVec3(shader_id, "object_color", vec3(1, 0, 0));
    SetUniformMat4(shader_id, "model_matrix", car);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    mat4 window2 = translate(mat4(1.0f), sizeInc * vec3(2.25f + carMove.x, 3.25, -7.5 + carMove.z)) *
                   scale(mat4(1.0f), sizeInc * vec3(3, 0.3, 0.1f));
    car = reposition * window2;
    SetUniformVec3(shader_id, "object_color", vec3(1, 0, 0));
    SetUniformMat4(shader_id, "model_matrix", car);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    mat4 back = translate(mat4(1.0f), sizeInc * vec3(2.25f + carMove.x, 2.5, -1.5 + carMove.z)) *
                scale(mat4(1.0f), sizeInc * vec3(3, 1.75, 0.1f));
    car = reposition * back;
    SetUniformVec3(shader_id, "object_color", vec3(1, 0, 0));
    SetUniformMat4(shader_id, "model_matrix", car);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    mat4 top = translate(mat4(1.0f), sizeInc * vec3(2.25f + carMove.x, 3.4, -4.5 + carMove.z)) *
               scale(mat4(1.0f), sizeInc * vec3(3, 0.1, 6.0f));
    car = reposition * top;
    SetUniformVec3(shader_id, "object_color", vec3(1, 0, 1));
    SetUniformMat4(shader_id, "model_matrix", car);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    glBindTexture(GL_TEXTURE_2D, tireText);
    glBindVertexArray(vaos);
    mat4 wheel1 = translate(mat4(1.0f), sizeInc * vec3(4.5f + carMove.x, 0.5f, -7.0f + carMove.z)) *
                  rotate(mat4(1.0f), radians(rotX), sizeInc * vec3(1, 0, 0)) *
                  scale(mat4(1.0f), sizeInc * vec3(0.5f, 1.0f, 1.0f));
    car = reposition * wheel1;
    SetUniformVec3(shader_id, "object_color", vec3(50 / 255.0, 50 / 255.0, 50 / 255.0));
    SetUniformMat4(shader_id, "model_matrix", car);
    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
    
    mat4 wheel2 = translate(mat4(1.0f), sizeInc * vec3(4.5f + carMove.x, 0.5f, -3.0f + carMove.z)) *
                  rotate(mat4(1.0f), radians(rotX), sizeInc * vec3(1, 0, 0)) *
                  scale(mat4(1.0f), sizeInc * vec3(0.5f, 1.0f, 1.0f));
    car = reposition * wheel2;
    SetUniformVec3(shader_id, "object_color", vec3(50 / 255.0, 50 / 255.0, 50 / 255.0));
    SetUniformMat4(shader_id, "model_matrix", car);
    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
    
    mat4 wheel3 = translate(mat4(1.0f), sizeInc * vec3(0 + carMove.x, 0.5f, -7.0f + carMove.z)) *
                  rotate(mat4(1.0f), radians(rotX), sizeInc * vec3(1, 0, 0)) *
                  scale(mat4(1.0f), sizeInc * vec3(0.5f, 1.0f, 1.0f));
    car = reposition * wheel3;
    SetUniformVec3(shader_id, "object_color", vec3(50 / 255.0, 50 / 255.0, 50 / 255.0));
    SetUniformMat4(shader_id, "model_matrix", car);
    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
    
    mat4 wheel4 = translate(mat4(1.0f), sizeInc * vec3(0 + carMove.x, 0.5f, -3.0f + carMove.z)) *
                  rotate(mat4(1.0f), radians(rotX), sizeInc * vec3(1, 0, 0)) *
                  scale(mat4(1.0f), sizeInc * vec3(0.5f, 1.0f, 1.0f));
    car = reposition * wheel4;
    SetUniformVec3(shader_id, "object_color", vec3(50 / 255.0, 50 / 255.0, 50 / 255.0));
    SetUniformMat4(shader_id, "model_matrix", car);
    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
    
}

int main(int argc, char *argv[]) {
    if (!InitContext()) return -1;
    
    // background
    glClearColor(0.41f, 0.44f, 0.62f, 1.0f);
    
    GLuint shaderScene = compileAndLinkShaders(SCENE_VERT, SCENE_FRAG);
    GLuint shaderShadow = compileAndLinkShaders(SHADOW_VERT, SHADOW_FRAG);
    GLuint shaderSkybox = compileAndLinkShaders(SKYBOX_VERT, SKYBOX_FRAG);
    
    // Load Textures
    GLuint bushTextureID = loadTexture(PATH_PREFIX "assets/textures/bush.jpg");
    GLuint leavesTextureID = loadTexture(PATH_PREFIX "assets/textures/leaves.png");
    GLuint woodTextureID = loadTexture(PATH_PREFIX "assets/textures/wood.jpg");
    GLuint roadTextureID = loadTexture(PATH_PREFIX "assets/textures/dirtroad.jpg");
    GLuint dirtTextureID = loadTexture(PATH_PREFIX "assets/textures/dirt.png");
    GLuint carTextureID = loadTexture(PATH_PREFIX "assets/textures/car.jpg");
    GLuint tireTextureID = loadTexture(PATH_PREFIX "assets/textures/tire.jpg");
    
    vector<std::string> skyFaces{
            PATH_PREFIX "assets/textures/skybox/px.jpg",  // right
            PATH_PREFIX "assets/textures/skybox/nx.jpg",  // left
            PATH_PREFIX "assets/textures/skybox/py.jpg",  // top
            PATH_PREFIX "assets/textures/skybox/ny.jpg",  // bottom
            PATH_PREFIX "assets/textures/skybox/pz.jpg",  // front
            PATH_PREFIX "assets/textures/skybox/nz.jpg"   // back
    };
    GLuint cubemapTexture = loadCubemap(skyFaces);
    
    glUseProgram(shaderSkybox);
    vec3 lightColor = vec3(1.0f, 1.0f, 1.0f); // Used for both the scene shader and the skybox shader
    SetUniformVec3(shaderSkybox, "lightColor", lightColor);
    
    glUseProgram(shaderScene);
    GLuint textureflag = glGetUniformLocation(shaderScene, "useTexture");
    GLint p_array[1];
    GLint *currentvalue = p_array;
    
    GLuint shadowflag = glGetUniformLocation(shaderScene, "useShadow");
    GLint p_arrayS[1];
    GLint *currentvalueS = p_arrayS;
    
    GLuint lightFlag = glGetUniformLocation(shaderScene, "lightsOn");
    bool toggleLights = true;
    glUniform1i(lightFlag, toggleLights);
    
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
    // The perfomance slows when cameraPos holds negative values, hence why it's fairly big
    vec3 cameraPosition(0.6f, 10.0f, 2000.0f);
    vec3 cameraPositionWalking(0.6f, 3.0f, 2000.0f);
    vec3 cameraLookAt(0.0f, 0.0f, -1.0f);
    vec3 cameraUp(0.0f, 1.0f, 0.0f);
    vec3 carMove(0, 0, 1980.0f);
    
    // Other camera parameters
    float cameraSpeed = 0.1f;
    float cameraFastSpeed = 2 * cameraSpeed;
    float cameraHorizontalAngle = 90.0f;
    float cameraVerticalAngle = 0.0f;
    fov = 70.0f;
    bool cameraFirstPerson = true;
    
    int selection = 0;
    float intensity = 0.2f;
    
    
    // Set projection matrix for shader, this won't change
    mat4 projectionMatrix = glm::perspective(70.0f,           // field of view in degrees
                                             WIDTH * 1.0f / HEIGHT, // aspect ratio
                                             0.01f, 1000.0f);  // near and far (near > 0)
    
    // Set initial view matrix on both shaders
    mat4 viewMatrix = lookAt(cameraPosition,                // eye
                             cameraPosition + cameraLookAt, // center
                             cameraUp);                     // up
    
    // Set projection matrix on both shaders
    SetUniformMat4(shaderScene, "projection_matrix", projectionMatrix);
    
    // Set view matrix on both shaders
    SetUniformMat4(shaderScene, "view_matrix", viewMatrix);
    
    
    float lightAngleOuter = 45.0;
    float lightAngleInner = 35.0;
    // Set light cutoff angles on scene shader
    SetUniform1fValue(shaderScene, "light_cutoff_inner", cos(radians(lightAngleInner)));
    SetUniform1fValue(shaderScene, "light_cutoff_outer", cos(radians(lightAngleOuter)));
    
    // Set light color on scene shader
    SetUniformVec3(shaderScene, "light_color", vec3(1.0, 1.0, 1.0));
    SetUniformVec3(shaderScene, "light_color2", vec3(1.0, 1.0, 1.0)); // Set light color on light shader
    
    // Set object color on scene shader
    SetUniformVec3(shaderScene, "object_color", vec3(1.0, 1.0, 1.0));
    
    GLuint vao = createTexturedCubeVAO();
    GLuint sphereVAO = createSphereObject();
    GLuint skyboxVAO = createSkyboxObject();
    
    // For frame time
    float lastFrameTime = glfwGetTime();
    double lastMousePosX, lastMousePosY;
    glfwGetCursorPos(window, &lastMousePosX, &lastMousePosY);
    
    // Other OpenGL states to set once
    glEnable(GL_DEPTH_TEST);
    
    glBindVertexArray(vao);
    
    int previousTstate = GLFW_RELEASE;
    int previousZstate = GLFW_RELEASE;
    int previousLstate = GLFW_RELEASE;
    int lastCState = GLFW_RELEASE;
    int lastMouseLeftState = GLFW_RELEASE;
    
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
        vec3 lightPosition = vec3(carMove.x, 1.0f, carMove.z + 1.5f); // the location of the light in 3D space
        vec3 lightFocus(0.0, 0.0, -1.0);      // the point in 3D space the light "looks" at
        vec3 lightDirection = normalize(lightFocus - lightPosition);
        
        float lightNearPlane = 1.0f;
        float lightFarPlane = 200.0f;
        
        mat4 lightProjectionMatrix = frustum(-1.0f, 1.0f, -1.0f, 1.0f, lightNearPlane, lightFarPlane);
//                perspective(20.0f, (float) DEPTH_MAP_TEXTURE_SIZE / (float) DEPTH_MAP_TEXTURE_SIZE, lightNearPlane,
//                            lightFarPlane);
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
        
        // Light parameters for point light (light two)
        vec3 lightPosition2 = vec3(1 + carMove.x, -0, 0.0f + carMove.z); // the location of the light in 3D space
        vec3 lightFocus2(0.0, -1.0, 10.0 + carMove.z);      // the point in 3D space the light "looks" at
        vec3 lightDirection2 = normalize(lightFocus2 - lightPosition2);
        
        float lightNearPlane2 = 0.0f; //1
        float lightFarPlane2 = 15.0f; //180
        
        mat4 lightProjectionMatrix2 = frustum(-1.0f, 1.0f, -1.0f, 1.0f, lightNearPlane2, lightFarPlane2);
        mat4 lightViewMatrix2 = lookAt(lightPosition2, lightFocus2, vec3(0.0f, 1.0f, 0.0f));
        mat4 lightSpaceMatrix2 = lightProjectionMatrix2 * lightViewMatrix2;
        
        // Set light far and near planes on scene shader
        SetUniform1Value(shaderScene, "light_near_plane2", lightNearPlane2);
        SetUniform1Value(shaderScene, "light_far_plane2", lightFarPlane2);
        
        SetUniformVec3(shaderScene, "light_position2", lightPosition2); // Set light position on scene shader
        SetUniformVec3(shaderScene, "light_direction2", lightDirection2); // Set light direction on scene shader
        SetUniform1Value(shaderScene, "useCarLight", carLight);
        SetUniformVec3(shaderScene, "light_color2", vec3(1.0, 1.0, 1.0)); // Set light color on light shader
        
        // Night and Day Timer
        SetUniform1fValue(shaderScene, "intensity", intensity); // Set initial intensity
        float skyStrength = 1.0f;
        if (glfwGetTime() <= 5) {
            skyStrength = 0.2f;
            SetUniform1fValue(shaderScene, "intensity", 0.2);
        } else if (glfwGetTime() <= 5.5 || glfwGetTime() >= 21.5) {
            skyStrength = 0.25f;
            SetUniform1fValue(shaderScene, "intensity", 0.25);
        } else if (glfwGetTime() <= 6 || glfwGetTime() >= 21) {
            skyStrength = 0.3f;
            SetUniform1fValue(shaderScene, "intensity", 0.3);
        } else if (glfwGetTime() <= 6.5 || glfwGetTime() >= 20.5) {
            skyStrength = 0.35f;
            SetUniform1fValue(shaderScene, "intensity", 0.35);
        } else if (glfwGetTime() <= 7 || glfwGetTime() >= 20) {
            skyStrength = 0.4f;
            SetUniform1fValue(shaderScene, "intensity", 0.4);
        } else if (glfwGetTime() <= 7.5 || glfwGetTime() >= 19.5) {
            skyStrength = 0.45f;
            SetUniform1fValue(shaderScene, "intensity", 0.45);
        } else if (glfwGetTime() <= 8 || glfwGetTime() >= 19) {
            skyStrength = 0.5f;
            SetUniform1fValue(shaderScene, "intensity", 0.5);
        } else if (glfwGetTime() <= 8.5 || glfwGetTime() >= 18.5) {
            skyStrength = 0.55f;
            SetUniform1fValue(shaderScene, "intensity", 0.55);
        } else if (glfwGetTime() <= 9 || glfwGetTime() >= 18) {
            skyStrength = 0.6f;
            SetUniform1fValue(shaderScene, "intensity", 0.6);
        } else if (glfwGetTime() <= 9.5 || glfwGetTime() >= 17.5) {
            skyStrength = 0.65f;
            SetUniform1fValue(shaderScene, "intensity", 0.65);
        } else if (glfwGetTime() <= 10 || glfwGetTime() >= 17) {
            skyStrength = 0.7f;
            SetUniform1fValue(shaderScene, "intensity", 0.7);
        } else if (glfwGetTime() <= 10.5 || glfwGetTime() >= 16.5) {
            skyStrength = 0.75f;
            SetUniform1fValue(shaderScene, "intensity", 0.75);
        } else if (glfwGetTime() <= 11 || glfwGetTime() >= 16) {
            skyStrength = 0.8f;
            SetUniform1fValue(shaderScene, "intensity", 0.8);
        } else if (glfwGetTime() <= 11.5 || glfwGetTime() >= 15.5) {
            skyStrength = 0.85f;
            SetUniform1fValue(shaderScene, "intensity", 0.85);
        } else if (glfwGetTime() <= 12 || glfwGetTime() >= 15) {
            skyStrength = 0.9f;
            SetUniform1fValue(shaderScene, "intensity", 0.9);
        } else if (glfwGetTime() <= 12.5 || glfwGetTime() >= 14.5) {
            skyStrength = 0.95f;
            SetUniform1fValue(shaderScene, "intensity", 0.95);
        } else {
            skyStrength = 1.0f;
            SetUniform1fValue(shaderScene, "intensity", 1.0);
        }
        
        if (glfwGetTime() >= 25) {
            glfwSetTime(0.0);
        }
        
        // Set model matrix and send to both shaders
        mat4 modelMatrix = mat4(1.0f);
        
        SetUniformMat4(shaderScene, "model_matrix", modelMatrix);
        SetUniformMat4(shaderShadow, "model_matrix", modelMatrix);
        
        // Set the view matrix for first person camera and send to both shaders
        viewMatrix = lookAt(cameraPosition, cameraPosition + cameraLookAt, cameraUp);
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
            
            renderScene(shaderShadow, vao, sphereVAO, cameraPosition.z, roadTextureID, dirtTextureID, woodTextureID,
                        leavesTextureID, carTextureID, tireTextureID, carMove);
            
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
            glClearColor(0.05f, 0.07f, 0.11f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            // Bind depth map texture: not needed, by default it is active
            //glActiveTexture(GL_TEXTURE0);
            
            
            // Draw textured geometry
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, depth_map_texture);
            
            // Draw textured geometry
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, roadTextureID);
            // Bind geometry
            
            GLuint worldMatrixLocation = glGetUniformLocation(shaderScene, "model_matrix");
            // Bind geometry
            glBindVertexArray(vao);
            
            renderScene(shaderScene, vao, sphereVAO, cameraPosition.z, roadTextureID, dirtTextureID, woodTextureID,
                        leavesTextureID, carTextureID, tireTextureID, carMove);
            
            // Unbind geometry
            glBindVertexArray(0);
        }
        
        // Draw skybox last for optimization (hidden portions won't be rendered)
        glUseProgram(shaderSkybox);
        SetUniformMat4(shaderSkybox, "view", viewMatrix);
        SetUniformMat4(shaderSkybox, "projection", projectionMatrix);
        
        // Apply daytime light changes to sky
        glUniform1f(glGetUniformLocation(shaderSkybox, "ambientStrength"), skyStrength);
        
        glDepthFunc(GL_LEQUAL); // Change depth function so that the skybox's maximmum depth value gets rendered
        glBindVertexArray(skyboxVAO);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthFunc(GL_LESS); // Back to default
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        
        // Handle inputs
        {
            // Escape
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
                glfwSetWindowShouldClose(window, true);
            
            
            // Toggle texture
            if (previousTstate == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
                glGetUniformiv(shaderScene, textureflag, currentvalue);
                if (currentvalue[0] == 1) {
                    glUseProgram(shaderScene);
                    glUniform1i(textureflag, 0);
                } else {
                    
                    glUseProgram(shaderScene);
                    glUniform1i(textureflag, 1);
                }
            }
            previousTstate = glfwGetKey(window, GLFW_KEY_T);
            
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
            
            // Toggle lights
            if (previousLstate == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
                carLight = !carLight;
            }
            previousLstate = glfwGetKey(window, GLFW_KEY_L);
            
            double mousePosX, mousePosY;
            glfwGetCursorPos(window, &mousePosX, &mousePosY);
            
            double dx = mousePosX - lastMousePosX;
            double dy = mousePosY - lastMousePosY;
            
            lastMousePosX = mousePosX;
            lastMousePosY = mousePosY;
            
            // Convert to spherical coordinates
            const float cameraAngularSpeed = 60.0f;
            cameraHorizontalAngle -= dx * cameraAngularSpeed * 0.015;
            cameraVerticalAngle -= dy * cameraAngularSpeed * 0.015;
            
            // Clamp vertical angle to [-85, 85] degrees
            cameraVerticalAngle = std::max(-85.0f, std::min(85.0f, cameraVerticalAngle));
            if (cameraHorizontalAngle > 360) {
                cameraHorizontalAngle -= 360;
            } else if (cameraHorizontalAngle < -360) {
                cameraHorizontalAngle += 360;
            }
            
            float theta = radians(cameraHorizontalAngle);
            float phi = radians(cameraVerticalAngle);
            
            
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
            
            //Change between aerial and person view
            if (lastCState == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
                
                if (camNum == 1) {
                    camNum = 2;
                } else if (camNum == 2) {
                    camNum = 3;
                } else if (camNum == 3) {
                    camNum = 4;
                } else if (camNum == 4) {
                    camNum = 1;
                }
            }
            lastCState = glfwGetKey(window, GLFW_KEY_C);
            
            // Free camera, default
            if (selection == 0) {
                
                cameraPosition = cameraPositionWalking;
                
                bool fastCam = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                               glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
                float currentCameraSpeed = (fastCam) ? cameraFastSpeed : cameraSpeed;
                
                glfwGetCursorPos(window, &mousePosX, &mousePosY);
                
                dx = mousePosX - lastMousePosX;
                dy = mousePosY - lastMousePosY;
                
                lastMousePosX = mousePosX;
                lastMousePosY = mousePosY;
                
                // Convert to spherical coordinates
                cameraHorizontalAngle -= dx * cameraAngularSpeed;
                cameraVerticalAngle -= dy * cameraAngularSpeed;
                
                // Clamp vertical angle to [-85, 85] degrees
                cameraVerticalAngle = std::max(-85.0f, std::min(85.0f, cameraVerticalAngle));
                if (cameraHorizontalAngle > 360) {
                    cameraHorizontalAngle -= 360;
                } else if (cameraHorizontalAngle < -360) {
                    cameraHorizontalAngle += 360;
                }
                
                theta = radians(cameraHorizontalAngle);
                phi = radians(cameraVerticalAngle);
                
                cameraLookAt = vec3(cosf(phi) * cosf(theta), sinf(phi), -cosf(phi) * sinf(theta));
                vec3 cameraSideVector = glm::cross(cameraLookAt, vec3(0.0f, 1.0f, 0.0f));
                
                glm::normalize(cameraSideVector);
                
                if (camNum == 1) { cameraPosition = vec3(0.0f + carMove.x, 3.0f, 0.0f + carMove.z); }
                else if (camNum == 2) { cameraPosition = vec3(0.0f + carMove.x, 3.25f, 5.25f + carMove.z); }
                else if (camNum == 3) { cameraPosition = vec3(0.0f + carMove.x, 8.0f, 20.0f + carMove.z); }
                else if (camNum == 4) { cameraPosition = vec3(0.0f + carMove.x, 30.0f, 20.0f + carMove.z); }
                
                
                if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) // move camera to the left
                {
                    cameraPosition -= cameraSideVector * currentCameraSpeed;
                    
                    carMove -= cameraSideVector * currentCameraSpeed;
                }
                
                if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) // move camera to the right
                {
                    cameraPosition += cameraSideVector * currentCameraSpeed;
                    
                    carMove += cameraSideVector * currentCameraSpeed;
                }
                
                if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) // move camera backward
                {
                    vec3 moveDirection = vec3(cameraLookAt.x, 0.0f, cameraLookAt.z);
                    cameraPosition -= moveDirection * currentCameraSpeed;
                    
                    carMove -= moveDirection * currentCameraSpeed;
                    
                    rotX -= 5.0f; // Car wheels rotation
                }
                
                if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) // move camera forward
                {
                    vec3 moveDirection = vec3(cameraLookAt.x, 0.0f, cameraLookAt.z);
                    cameraPosition += moveDirection * currentCameraSpeed;
                    
                    carMove += moveDirection * currentCameraSpeed;
                    
                    rotX += 5.0f; // Car wheels rotation
                    
                    cout << "cameraPos.z: " << cameraPosition.z << "\t Change in time: " << dt << "\n";
                }
                
                // walking boundaries
                if (cameraPosition.x > 2.0f)
                    cameraPosition.x = 2.0f;
                if (cameraPosition.x < -2.0f)
                    cameraPosition.x = -2.0f;
                if (carMove.x > 2.0f)
                    carMove.x = 2.0f;
                if (carMove.x < -2.0f)
                    carMove.x = -2.0f;
                
                
                viewMatrix = mat4(1.0);
                
                if (cameraFirstPerson) {
                    viewMatrix = lookAt(cameraPosition, cameraPosition + cameraLookAt, cameraUp);
                } else {
                    float radius = 5.0f;
                    position = cameraPosition - radius * cameraLookAt;
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
                
                glfwGetCursorPos(window, &mousePosX, &mousePosY);
                
                dx = mousePosX - lastMousePosX;
                dy = mousePosY - lastMousePosY;
                
                lastMousePosX = mousePosX;
                lastMousePosY = mousePosY;
                
                // Convert to spherical coordinates
                cameraHorizontalAngle -= dx * cameraAngularSpeed;
                cameraVerticalAngle -= dy * cameraAngularSpeed;
                
                // Clamp vertical angle to [-85, 85] degrees
                cameraVerticalAngle = std::max(-85.0f, std::min(85.0f, cameraVerticalAngle));
                if (cameraHorizontalAngle > 360) {
                    cameraHorizontalAngle -= 360;
                } else if (cameraHorizontalAngle < -360) {
                    cameraHorizontalAngle += 360;
                }
                
                theta = radians(cameraHorizontalAngle);
                phi = radians(cameraVerticalAngle);
                
                cameraLookAt = vec3(cosf(phi) * cosf(theta), sinf(phi), -cosf(phi) * sinf(theta));
                vec3 cameraSideVector = glm::cross(cameraLookAt, vec3(0.0f, 1.0f, 0.0f));
                
                glm::normalize(cameraSideVector);
                
                viewMatrix = mat4(1.0);
                
                if (cameraFirstPerson) {
                    viewMatrix = lookAt(cameraPosition, cameraPosition + cameraLookAt, cameraUp);
                } else {
                    float radius = 5.0f;
                    position = cameraPosition - radius * cameraLookAt;
                    viewMatrix = lookAt(position, position + cameraLookAt, cameraUp);
                }
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
    window = glfwCreateWindow(WIDTH, HEIGHT, "Comp371 - Procedural World", NULL, NULL);
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

// Loads the 6 cube face images, binds and generates the resulting cubemap texture, and returns its ID
GLuint loadCubemap(vector<std::string> faces) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    
    int width, height, nrChannels;
    
    // The cube map texture enums can be incremented in order (right, left, top, bottom, front, back)
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        } else {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    
    return textureID;
}

// Generates VAO & VBO for the skybox/cubemap and returns VAO identifier
GLuint createSkyboxObject() {
    float skyboxVertices[] = {
            // positions
            -1.0f, 1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            
            -1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,
            
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            
            -1.0f, -1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,
            
            -1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, -1.0f,
            
            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, 1.0f
    };
    
    GLuint skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
    
    return skyboxVAO;
}

GLuint createTexturedCubeVAO() {
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


GLuint createSphereObject() {
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

int lastChunkID = -100;

void renderScene(GLuint shader, GLuint texturedCubeVAO, GLuint sphereVAO, float cameraPosZ, GLuint roadTextureID,
                 GLuint dirtTextureID, GLuint woodTextureID, GLuint leavesTextureID,
                 GLuint carTextureID, GLuint tireTextureID, vec3 carMove) {
    
    
    int currentChunkID = static_cast<int>(floor((cameraPosZ - 50) / 100));
    
    if (lastChunkID != currentChunkID) {
        cout << "CHANGED ID: " << currentChunkID << "\n";
    }
    lastChunkID = currentChunkID;
    
    // Only 5 chunks in total are rendered each frame (number of chunks needs to be odd for proper positioning)
    for (int i = currentChunkID - 2; i <= currentChunkID + 2; i++) {
        // All previously rendered chunks are saved to be able to go back to same scene
        if (!chunksByPosition.count(i)) {
            cout << "POPULATED ID: " << i << "\n";
            chunksByPosition.insert(make_pair(i, WorldChunk(i)));
        }
        
        WorldChunk chunk = chunksByPosition.at(i);
        
        // Floor
        glBindTexture(GL_TEXTURE_2D, dirtTextureID);
        worldMatrix = chunk.getGroundMatrix();
        setWorldMatrix(shader, worldMatrix);
        SetUniformVec3(shader, "object_color", vec3(0.38f, 0.63f, 0.33f)); // Green
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        // Road
        glBindTexture(GL_TEXTURE_2D, roadTextureID);
        worldMatrix = chunk.getRoadMatrix();
        setWorldMatrix(shader, worldMatrix);
        SetUniformVec3(shader, "object_color", vec3(0.5f, 0.5f, 0.5f)); // Gray
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        for (auto tree: chunk.bigTreePositions) {
            drawTree(shader, tree.z, tree.x, 0.0f, 1, woodTextureID, leavesTextureID);
        }
        
        for (auto tree: chunk.smallTreePositions) {
            drawTree(shader, tree.z, tree.x, 0.0f, 2, woodTextureID, leavesTextureID);
        }
        
        glBindVertexArray(sphereVAO);
        for (auto bush: chunk.bushPositions) {
            drawBush(shader, bush.z, bush.x, 0.0f, 1, leavesTextureID);
        }
        
        glBindVertexArray(texturedCubeVAO);
    }
    
    drawCar(shader, sphereVAO, carMove, carTextureID, tireTextureID);
}