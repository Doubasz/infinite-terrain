

#include "glad/glad.h" 
#include "SkyBox.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


SkyBox::SkyBox(const std::string& vertexPath, const std::string& fragPath){
    cubeMesh = createCubeMesh(1.0);
    shader = new Shader(vertexPath, fragPath);
    cubemapTexture = -1;
}

SkyBox::SkyBox(const std::vector<std::string>& faces, const std::string& vertexPath, const std::string& fragPath){
    cubeMesh = createCubeMesh(1.0);
    shader = new Shader(vertexPath, fragPath);
    cubemapTexture = loadCubemap(faces);
}

SkyBox::~SkyBox(){
    delete cubeMesh;
    delete shader;
    glDeleteTextures(1, &cubemapTexture);
}


void SkyBox::draw(const glm::mat4& cameraView, const glm::mat4& projection, bool img){
    glDepthFunc(GL_LEQUAL);

    shader->use();

    glm::mat4 view = cameraView;
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);

    glBindVertexArray(cubeMesh->VAO);

    if(img){
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        shader->setInt("skybox", 0);
    }

    glDrawElements(GL_TRIANGLES, cubeMesh->indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);
}


unsigned int SkyBox::loadCubemap(const std::vector<std::string>& faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                        0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cerr << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}