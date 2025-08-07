#include "stdafx.h"
#include "Model.h"
#include "../../Utilities/TGA.h"
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
        return false;
    }

    vertices.clear();
    indices.clear();

    std::string line;
    int numVertices = 0;

    while (std::getline(file, line)) {
        if (line.find("NrVertices:") != std::string::npos) {
            sscanf(line.c_str(), "NrVertices: %d", &numVertices);
            break;
        }
    }

    while ((int)vertices.size() < numVertices && std::getline(file, line)) {
        if (line.find("pos:") == std::string::npos) continue;

        Vertex vertex;
        Vector3 normal, binormal, tangent;

        int result = sscanf(line.c_str(),
            "%*d. pos:[%f, %f, %f]; norm:[%f, %f, %f]; binorm:[%f, %f, %f]; tgt:[%f, %f, %f]; uv:[%f, %f];",
            &vertex.pos.x, &vertex.pos.y, &vertex.pos.z,
            &normal.x, &normal.y, &normal.z,
            &binormal.x, &binormal.y, &binormal.z,
            &tangent.x, &tangent.y, &tangent.z,
            &vertex.uv.x, &vertex.uv.y);

        if (result == 14) {
            vertex.color.x = (normal.x + 1.0f) * 0.5f;
            vertex.color.y = (normal.y + 1.0f) * 0.5f;
            vertex.color.z = (normal.z + 1.0f) * 0.5f;

            vertices.push_back(vertex);
        }
    }

    int numIndices = 0;
    while (std::getline(file, line)) {
        if (line.find("NrIndices:") != std::string::npos) {
            sscanf(line.c_str(), "NrIndices: %d", &numIndices);
            break;
        }
    }

    while ((int)indices.size() < numIndices && std::getline(file, line)) {
        int i0, i1, i2;
        if (sscanf(line.c_str(), "%*d. %d, %d, %d", &i0, &i1, &i2) == 3) {
            indices.push_back((GLushort)i0);
            indices.push_back((GLushort)i1);
            indices.push_back((GLushort)i2);
        }
    }

    file.close();

    return !vertices.empty() && !indices.empty();
}

bool Model::LoadTexture(const char* filename) {
    int width, height, bpp;
    char* textureData = LoadTGA(filename, &width, &height, &bpp);

    if (!textureData) {
        return false;
    }

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    GLenum format = (bpp == 24) ? GL_RGB : GL_RGBA;

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height,
        0, format, GL_UNSIGNED_BYTE, textureData);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    delete[] textureData;
    return true;
}

void Model::CreateBuffers() {
    if (vertices.empty() || indices.empty()) {
        return;
    }

    vertexCount = (int)vertices.size();
    indexCount = (int)indices.size();

    glGenBuffers(1, &vboId);
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &iboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), indices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Model::Draw() {
    if (textureId) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureId);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(Vector3)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(Vector3) * 2));

    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, 0);

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
