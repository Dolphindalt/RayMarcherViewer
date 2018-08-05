#ifndef CAMERA_H
#define CAMERA_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

class Camera
{
public:
    Camera();

    glm::vec3 get_camera_position();

    void w();
    void a();
    void s();
    void d();
    void q();
    void e();
private:
    glm::vec3 _camera_position = glm::vec3(0.0f, 0.0f, -5.0f);
};

#endif