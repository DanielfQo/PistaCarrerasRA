#ifndef MODEL_RENDERER_H
#define MODEL_RENDERER_H

#include <string>
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <opencv2/opencv.hpp>

extern const char* vertexShaderSource_model;
extern const char* fragmentShaderSource_model;

class ModelRenderer {
public:
    ModelRenderer(const std::string& path);
    void SetModelMatrix(const glm::mat4& model);
    void SetViewProjection(const glm::mat4& view, const glm::mat4& projection);
    void Draw();

private:
    void LoadModel(const std::string& path);
    void SetupMesh();
    GLuint LoadTexture(const std::string& filename);
    GLuint LoadTextureEXscan(const std::string& objPath);
    void LoadModelEXscan(const std::string& path);

    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    GLuint VAO, VBO, EBO;
    GLuint shaderProgram;
    GLuint textureID = 0;

    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;

};

glm::mat4 PoseToGLMMat(const cv::Mat& rvec, const cv::Mat& tvec);

glm::mat4 BuildProjectionMatrix(const cv::Mat& cameraMatrix, float width, float height, float near, float far);
void UpdateBackgroundTexture(const cv::Mat& frame);
void DrawBackground();

#endif 
