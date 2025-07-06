#pragma once
#include "Vertex.h"
#include "../Utilities/Math.h"
#include <vector>

// Forward declare LoadTGA function
extern char* LoadTGA(const char* szFileName, int* width, int* height, int* bpp);

class Model {
public:
    std::vector<Vertex> vertices;  // Chứa data đã sẵn sàng để render
    
    GLuint vboId;
    GLuint iboId;
    GLuint textureId;
    int vertexCount;
    int indexCount;
    
    Model();
    ~Model();
    
    bool LoadFromNFG(const char* filename);  // Load NFG format
    bool LoadTexture(const char* filename);
    void CreateBuffers();
    void Draw();
    void Cleanup();
}; 