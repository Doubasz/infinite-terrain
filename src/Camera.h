#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <iostream>

enum CameraMode {
    FREE_MODE,
    PLAYER_MODE
};

struct Frustum {
    std::array<glm::vec4, 6> planes; // left, right, top, bottom, near, far
};

class Camera {
private:
    // Orientation
    glm::vec3 position;
    glm::vec3 front, up, right;
    glm::vec3 worldUp;

    float yaw, pitch;

    glm::vec3 freeVelocity;

    CameraMode mode;

    bool firstMouse = true;
    

    
    void updateCameraVectors();

public:
    float movementSpeed;
    float mouseSensitivity;
    float fov;

    Camera();
    Camera(const glm::vec3 startPos);
    ~Camera();

    void setMode(CameraMode newMode);
    CameraMode getMode() const;

    void processKeyboard(int direction, float deltaTime);
    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch);

    void update();

    glm::mat4 getViewMatrix() const;
    glm::vec3 getPosition() const;
    glm::vec3 getFront() const { return front; }
    glm::vec3 getUp() const { return up; }
    glm::vec3 getRight() const { return right; }
    float getYaw() const { return yaw; }
    float getPitch() const { return pitch; }
};

Frustum extractFrustum(const glm::mat4& vp);
bool isInFrustum(const Frustum& f, const glm::vec3& min, const glm::vec3& max);


#endif
