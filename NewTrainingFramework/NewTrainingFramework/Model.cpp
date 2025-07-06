#include "stdafx.h"
#include "Model.h"
#include "../Utilities/TGA.h"
#include <fstream>
#include <sstream>

Model::Model() : vboId(0), iboId(0), textureId(0), vertexCount(0), indexCount(0) {
}

Model::~Model() {
    Cleanup();
}

bool Model::LoadFromNFG(const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Không thể mở file NFG: " << filename << std::endl;
        return false;
    }
    
    vertices.clear();
    
    std::string line;
    int numVertices = 0;
    
    // Đọc số lượng vertices
    if (std::getline(file, line)) {
        if (line.find("NrVertices:") != std::string::npos) {
            sscanf(line.c_str(), "NrVertices: %d", &numVertices);
            std::cout << "Loading " << numVertices << " vertices from NFG..." << std::endl;
        }
    }
    
    // Đọc từng vertex
    while (std::getline(file, line) && (int)vertices.size() < numVertices) {
        if (line.empty() || line.find("pos:") == std::string::npos) continue;
        
        Vertex vertex;
        Vector3 normal, binormal, tangent;
        
        // Parse line format: 
        // pos:[x,y,z]; norm:[x,y,z]; binorm:[x,y,z]; tgt:[x,y,z]; uv:[u,v];
        int result = sscanf(line.c_str(), 
            "%*d. pos:[%f, %f, %f]; norm:[%f, %f, %f]; binorm:[%f, %f, %f]; tgt:[%f, %f, %f]; uv:[%f, %f];",
            &vertex.pos.x, &vertex.pos.y, &vertex.pos.z,
            &normal.x, &normal.y, &normal.z,
            &binormal.x, &binormal.y, &binormal.z,
            &tangent.x, &tangent.y, &tangent.z,
            &vertex.uv.x, &vertex.uv.y);
        
        if (result == 14) {  // Successfully parsed all 14 values
            // Convert normal to color for visualization
            vertex.color.x = (normal.x + 1.0f) * 0.5f;
            vertex.color.y = (normal.y + 1.0f) * 0.5f;
            vertex.color.z = (normal.z + 1.0f) * 0.5f;
            
            vertices.push_back(vertex);
        }
    }
    
    file.close();
    std::cout << "Loaded NFG model: " << vertices.size() << " vertices" << std::endl;
    return vertices.size() > 0;
}

bool Model::LoadTexture(const char* filename) {
    int width, height, bpp;
    char* textureData = LoadTGA(filename, &width, &height, &bpp);
    
    if (!textureData) {
        std::cout << "Không thể load texture: " << filename << std::endl;
        return false;
    }
    
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    // Determine format based on bpp
    GLenum format = (bpp == 24) ? GL_RGB : GL_RGBA;
    
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 
                 0, format, GL_UNSIGNED_BYTE, textureData);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    std::cout << "Loaded texture: " << filename << " (" << width << "x" << height << ")" << std::endl;
    delete[] textureData;  // LoadTGA allocates with new[]
    return true;
}

void Model::CreateBuffers() {
    if (vertices.empty()) {
        std::cout << "No vertices to create buffers!" << std::endl;
        return;
    }
    
    vertexCount = (int)vertices.size();
    
    // Tạo indices cho triangles - NFG format có vertices sắp xếp theo triangles  
    std::vector<GLushort> indexData;
    
    // Draw mỗi 3 vertices liên tiếp như 1 triangle
    for (int i = 0; i < vertexCount - 2; i += 3) {
        if (i + 2 < vertexCount) {
            indexData.push_back((GLushort)i);
            indexData.push_back((GLushort)i + 1);
            indexData.push_back((GLushort)i + 2);
        }
    }
    indexCount = (int)indexData.size();
    
    // Create VBO
    glGenBuffers(1, &vboId);
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // Create IBO
    glGenBuffers(1, &iboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(GLushort), indexData.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    std::cout << "Created buffers: " << vertexCount << " vertices, " << indexCount << " indices" << std::endl;
}

void Model::Draw() {
    // Bind texture
    if (textureId) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureId);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
    
    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    
    // Color attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(Vector3)));
    
    // UV attribute
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(Vector3) + sizeof(Vector3)));
    
    // Solid triangle rendering  
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, 0);
    
    // Debug point cloud (comment out for solid model)
    // glDrawArrays(GL_POINTS, 0, vertexCount);
    
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    if (textureId) {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void Model::Cleanup() {
    if (vboId) {
        glDeleteBuffers(1, &vboId);
        vboId = 0;
    }
    if (iboId) {
        glDeleteBuffers(1, &iboId);
        iboId = 0;
    }
    if (textureId) {
        glDeleteTextures(1, &textureId);
        textureId = 0;
    }
} 