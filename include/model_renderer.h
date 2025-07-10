#ifndef MODEL_RENDERER_H
#define MODEL_RENDERER_H

#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

class ModelRenderer {
public:
    explicit ModelRenderer(const std::string& path);
    ~ModelRenderer();
    
    void Draw();
    void SetModelMatrix(const glm::mat4& model);
    void SetViewProjection(const glm::mat4& view, const glm::mat4& projection);

private:
    void LoadModel(const std::string& path);
    void SetupMesh();
    void Cleanup();

    unsigned int VAO, VBO, EBO;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;

    unsigned int shaderProgram;
};

#endif // MODEL_RENDERER_H