#pragma once
#include "Vertex.h"
#include "../../Utilities/Math.h"
#include <vector>

extern char* LoadTGA(const char* szFileName, int* width, int* height, int* bpp);

class Model {
public:
    std::vector<Vertex> vertices;
    std::vector<GLushort> indices;

    GLuint vboId;
    GLuint iboId;
    GLuint textureId;
    int vertexCount;
    int indexCount;
    
    Model();
    ~Model();
    
    bool LoadFromNFG(const char* filename);
    bool LoadTexture(const char* filename);
    void CreateBuffers();
    void Draw();
    void Cleanup();
}; 