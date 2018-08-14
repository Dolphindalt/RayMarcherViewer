#ifndef CAMERA_H
#define CAMERA_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

class Camera
{
public:
    Camera();

    glm::vec3 get_center();
    glm::vec3 get_eye();
    glm::vec3 get_up();
    float *getScrollFactor();
    float *getCameraSpeed();

    void scroll(float offset);
    void rotate(int xrel, int yrel);
    void move(float xrel, float yrel);
private:
    glm::vec3 _center = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 _eye = glm::vec3(2.598076f, 3.0f, 4.5f);
    glm::vec3 _up = glm::vec3(-0.25, 0.866025, -0.433013);
};

#endif