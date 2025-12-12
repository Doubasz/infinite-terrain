#ifndef SKYBOX_H
#define SKYBOX_H

#include "Mesh.h"
#include "Shader.h"
#include "Camera.h"
#include <vector>
#include <string>


class SkyBox{
private: 
    unsigned int cubemapTexture;
    Mesh* cubeMesh;
    Shader* shader;

    unsigned int loadCubemap(const std::vector<std::string>& faces);

public:
    SkyBox(const std::vector<std::string>& faces, const std::string& vertexPath, const std::string& fragPath);
    SkyBox(const std::string& vertexPath, const std::string& fragPath);
    ~SkyBox();

    void draw(const glm::mat4& cameraView, const glm::mat4& projection, bool img);

};


#endif