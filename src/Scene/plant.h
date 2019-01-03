//
// Created by dydxh on 1/3/19.
//

#ifndef TMPER_PLANT_H
#define TMPER_PLANT_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "../Resource/model.h"
#include "../Resource/texture.h"
#include "../config.h"
#include <vector>

struct Treeinfo{
    int chunk_number;
    GLfloat height;
    glm::vec3 location;
};

class Plants {
public:
    Shader shader, shadowshader;
    GLuint amount;
    glm::mat4 matrixs[MESH_SIZE * MESH_SIZE];
    Plants();
    ~Plants();
    void Initialize();
    void SetParam(const std::vector<Treeinfo>& places);
    void Draw(const glm::mat4& PVMatrix, const glm::mat4& lightSpaceMatrix, const Texture2D& BluredShadow);
    void GenerateShadow(const glm::mat4& lightSpaceMatrix);
};


#endif //TMPER_PLANT_H
