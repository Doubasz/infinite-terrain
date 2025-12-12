
#include "glad/glad.h"
#include "Shader.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>




Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath){
    std::string vertexCode = loadShaderSource(vertexPath);
    std::string fragmentCode = loadShaderSource(fragmentPath);
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();


    GLuint vertex, fragment;
    int success;
    char infoLog[512];

    // Vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");

    // Fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    // Cleaning up
    glDeleteShader(vertex);
    glDeleteShader(fragment);

}


void Shader::use() const{
    glUseProgram(ID);
}

void Shader::setBool(const std::string& name, bool value) const{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const{
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) const{
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) const{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

std::string Shader::loadShaderSource(const std::string& path){
    std::ifstream file;
    std::stringstream buff;

    file.open(path);
    if(!file.is_open()){
        std::cerr << "ERROR::SHADER::FILE NOT READ: " << path << std::endl;
    }
    buff << file.rdbuf();
    file.close();
    return buff.str();
}

void Shader::checkCompileErrors(unsigned int shader, const std::string &type) {
    int success;
    char infoLog[1024];
    if(type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if(!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " 
                      << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " 
                      << std::endl;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if(!success) {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " 
                      << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " 
                      << std::endl;
        }
    }
}


bool Shader::hasUniform(const std::string& name){
    return glGetUniformLocation(ID, name.c_str()) != -1;
}