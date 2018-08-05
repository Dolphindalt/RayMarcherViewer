#include "Camera.h"

using namespace glm;

const float camera_speed = 0.05f;

Camera::Camera()
{

}

vec3 Camera::get_camera_position()
{
    return _camera_position;
}

void Camera::w()
{
    _camera_position.y += camera_speed;
}

void Camera::a()
{
    _camera_position.x -= camera_speed;
}

void Camera::s()
{
    _camera_position.y -= camera_speed;
}

void Camera::d()
{
    _camera_position.x += camera_speed;
}

void Camera::q()
{
    _camera_position.z -= camera_speed;
}

void Camera::e()
{
    _camera_position.z += camera_speed;
}