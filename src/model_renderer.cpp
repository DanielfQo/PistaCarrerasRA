#include "model_renderer.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

// Shader sources
const char* vertexShaderSource = R"glsl(
#version 120
attribute vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)glsl";

const char* fragmentShaderSource = R"glsl(
#version 120
void main() {
    gl_FragColor = vec4(0.3, 0.7, 0.9, 1.0);
}
)glsl";

// Helper functions
static void checkGLError(const char* context) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error in " << context << ": " << err << std::endl;
    }
}

static bool checkShaderCompileStatus(GLuint shader) {
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation error:\n" << infoLog << std::endl;
        return false;
    }
    return true;
}

static bool checkProgramLinkStatus(GLuint program) {
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Program linking error:\n" << infoLog << std::endl;
        return false;
    }
    return true;
}

// Constructor
ModelRenderer::ModelRenderer(const std::string& path) 
    : VAO(0), VBO(0), EBO(0), shaderProgram(0) {
    LoadModel(path);
    
    // Shader setup
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    if (!checkShaderCompileStatus(vertexShader)) {
        glDeleteShader(vertexShader);
        return;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    if (!checkShaderCompileStatus(fragmentShader)) {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    if (!checkProgramLinkStatus(shaderProgram)) {
        return;
    }

    SetupMesh();
}

// Destructor
ModelRenderer::~ModelRenderer() {
    Cleanup();
}

void ModelRenderer::Cleanup() {
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (shaderProgram) glDeleteProgram(shaderProgram);
}

void ModelRenderer::LoadModel(const std::string& path) {
    std::cout << "Cargando modelo: " << path << std::endl;

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, 
        aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Assimp error: " << importer.GetErrorString() << std::endl;
        return;
    }

    if (scene->mNumMeshes == 0) {
        std::cerr << "Error: el modelo no contiene mallas." << std::endl;
        return;
    }

    aiMesh* mesh = scene->mMeshes[0];
    vertices.reserve(mesh->mNumVertices * 3);
    indices.reserve(mesh->mNumFaces * 3);

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        vertices.push_back(mesh->mVertices[i].x);
        vertices.push_back(mesh->mVertices[i].y);
        vertices.push_back(mesh->mVertices[i].z);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    std::cout << "Modelo cargado exitosamente. Vértices: " << mesh->mNumVertices << std::endl;
}

void ModelRenderer::SetupMesh() {
    if (vertices.empty() || indices.empty()) {
        std::cerr << "Error: No se puede crear el mesh, datos vacíos." << std::endl;
        return;
    }

    // Verifica que las funciones de OpenGL estén cargadas
    if (!glGenVertexArrays || !glGenBuffers) {
        std::cerr << "Error: Funciones de OpenGL no cargadas." << std::endl;
        return;
    }

    glGenVertexArrays(1, &VAO);
    checkGLError("glGenVertexArrays");
    
    glGenBuffers(1, &VBO);
    checkGLError("glGenBuffers VBO");
    
    glGenBuffers(1, &EBO);
    checkGLError("glGenBuffers EBO");

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void ModelRenderer::Draw() {
    if (shaderProgram == 0 || VAO == 0) {
        std::cerr << "Draw abortado: shader/VAO inválido." << std::endl;
        return;
    }

    glUseProgram(shaderProgram);

    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

    if (modelLoc != -1) glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
    if (viewLoc != -1) glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
    if (projLoc != -1) glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void ModelRenderer::SetModelMatrix(const glm::mat4& model) {
    modelMatrix = model;
}

void ModelRenderer::SetViewProjection(const glm::mat4& view, const glm::mat4& projection) {
    viewMatrix = view;
    projectionMatrix = projection;
}