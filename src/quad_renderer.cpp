#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "../include/quad_renderer.h"

GLuint quadVAO = 0, quadVBO = 0, quadShader = 0, quadTex = 0;

const char* quadVS = R"(#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
out vec2 uv;
void main(){
    uv = aUV;
    gl_Position = vec4(aPos, 0.0, 1.0);
})";

const char* quadFS = R"(#version 330 core
in vec2 uv;
out vec4 Frag;
uniform sampler2D tex;
void main(){
    Frag = texture(tex, uv);
})";

GLuint compile(GLenum type, const char* src){
    GLuint sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);
    GLint ok;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(sh, 512, nullptr, log);
        std::cerr << "Shader compile error:\n" << log << std::endl;
    }
    return sh;
}

void initQuad(){
    float verts[] = {
        -1,  1,  0, 1,
        -1, -1,  0, 0,
         1, -1,  1, 0,
        -1,  1,  0, 1,
         1, -1,  1, 0,
         1,  1,  1, 1
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    GLuint vs = compile(GL_VERTEX_SHADER, quadVS);
    GLuint fs = compile(GL_FRAGMENT_SHADER, quadFS);
    quadShader = glCreateProgram();
    glAttachShader(quadShader, vs);
    glAttachShader(quadShader, fs);
    glLinkProgram(quadShader);
    glDeleteShader(vs);
    glDeleteShader(fs);

    glGenTextures(1, &quadTex);
    glBindTexture(GL_TEXTURE_2D, quadTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
