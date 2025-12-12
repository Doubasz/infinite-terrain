
#include "Camera.h"



Camera::Camera() : position(glm::vec3(0.0f)), up(glm::vec3(0.0f, 1.0f, 0.0f)), yaw(-90.0f), pitch(0.0f),
    worldUp(glm::vec3(0.0f, 1.0f, 0.0f)), movementSpeed(50.0f), mouseSensitivity(0.1f), fov(45.0f), 
    freeVelocity(0.0f), mode(FREE_MODE)
{
    updateCameraVectors();
}

Camera::Camera(const glm::vec3 startPos) : Camera() {
    position = startPos;
}

Camera::~Camera(){

}

void Camera::updateCameraVectors(){
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);
    
    if(mode == FREE_MODE){
        right = glm::normalize(glm::cross(front, worldUp));
        up = glm::normalize(glm::cross(right, front));
    }
    
}

void Camera::setMode(CameraMode newMode){
    mode = newMode;
    freeVelocity = glm::vec3(0.0f);
}

CameraMode Camera::getMode() const{
    return mode;
}

glm::vec3 Camera::getPosition() const{
    return position;
}

glm::mat4 Camera::getViewMatrix() const{
    // Free camera
    return glm::lookAt(position, position + front, up);

}

void Camera::processKeyboard(int direction, float deltaTime){
    if (mode == FREE_MODE) {
        float vel = movementSpeed * deltaTime;
        
        if (direction == 0) position += front * vel;  // Z
        if (direction == 1) position -= front * vel;  // S
        if (direction == 2) position -= right * vel;  // Q
        if (direction == 3) position += right * vel;  // D
        if (direction == 4) position += worldUp * vel; // Space
        if (direction == 5) position -= worldUp * vel; // Shift

    }
}

void Camera::processMouseMovement(float xoffset, float yoffset, bool constrainPitch){
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;
    
    yaw += xoffset;
    pitch += yoffset;
    
    if (constrainPitch) {
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;
    }
    
    updateCameraVectors();
}

void Camera::update(){
    updateCameraVectors();
}





Frustum extractFrustum(const glm::mat4& vp) {
    Frustum f;
    // left plane
    f.planes[0] = glm::vec4(vp[0][3] + vp[0][0], vp[1][3] + vp[1][0], vp[2][3] + vp[2][0], vp[3][3] + vp[3][0]);
    // right plane
    f.planes[1] = glm::vec4(vp[0][3] - vp[0][0], vp[1][3] - vp[1][0], vp[2][3] - vp[2][0], vp[3][3] - vp[3][0]);
    // bottom plane
    f.planes[2] = glm::vec4(vp[0][3] + vp[0][1], vp[1][3] + vp[1][1], vp[2][3] + vp[2][1], vp[3][3] + vp[3][1]);
    // top plane
    f.planes[3] = glm::vec4(vp[0][3] - vp[0][1], vp[1][3] - vp[1][1], vp[2][3] - vp[2][1], vp[3][3] - vp[3][1]);
    // near plane
    f.planes[4] = glm::vec4(vp[0][3] + vp[0][2], vp[1][3] + vp[1][2], vp[2][3] + vp[2][2], vp[3][3] + vp[3][2]);
    // far plane
    f.planes[5] = glm::vec4(vp[0][3] - vp[0][2], vp[1][3] - vp[1][2], vp[2][3] - vp[2][2], vp[3][3] - vp[3][2]);

    // normalize planes
    for (auto& p : f.planes) {
        float length = glm::length(glm::vec3(p));
        p /= length;
    }
    return f;
}

bool isInFrustum(const Frustum& f, const glm::vec3& min, const glm::vec3& max) {
    // check AABB against planes
    for (const auto& plane : f.planes) {
        glm::vec3 positive = min;
        if (plane.x >= 0) positive.x = max.x;
        if (plane.y >= 0) positive.y = max.y;
        if (plane.z >= 0) positive.z = max.z;

        if (glm::dot(glm::vec3(plane), positive) + plane.w < 0) return false;
    }
    return true;
}