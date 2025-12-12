#pragma once
#include <glad/glad.h>
#include <string>
#include <unordered_map>
#include <iostream>
#include "stb_image.h"

class TextureManager {
public:
    // Load a texture from file and assign a name
    static bool loadTexture(const std::string& name, const std::string& filepath, bool flipVertically = true) {
        if (textures.count(name)) return true; // already loaded

        stbi_set_flip_vertically_on_load(flipVertically);
        int width, height, channels;
        unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);
        if (!data) {
            std::cerr << "Failed to load texture: " << filepath << "\n";
            return false;
        }

        GLuint texID;
        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_2D, texID);

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        GLenum format = GL_RGB;
        if (channels == 1) format = GL_RED;
        else if (channels == 3) format = GL_RGB;
        else if (channels == 4) format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);

        textures[name] = texID;
        return true;
    }

    // Retrieve texture ID by name
    static GLuint getTexture(const std::string& name) {
        if (!textures.count(name)) {
            std::cerr << "Texture not found: " << name << "\n";
            return 0;
        }
        return textures[name];
    }

    // Cleanup all loaded textures
    static void cleanup() {
        for (auto& [name, id] : textures) {
            glDeleteTextures(1, &id);
        }
        textures.clear();
    }

private:
    static inline std::unordered_map<std::string, GLuint> textures;
};
