#include "Camera.h"

#include <SDL2/SDL.h>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/scalar_multiplication.hpp>

using namespace glm;

extern glm::vec3 resolution;
extern int last_x, last_y;

float camera_speed = 1.0f;
float scroll_factor = 0.1f;
const double tau = 6.283185307179586;
double zoom = 1.0f;

Camera::Camera()
{

}

vec3 Camera::get_center()
{
    return _center;
}

vec3 Camera::get_eye()
{
    return _eye;
}

vec3 Camera::get_up()
{
    return _up;
}

void Camera::scroll(float offset)
{
    offset = (offset > 0) ? 1.0 + scroll_factor : 1.0 - scroll_factor;
    vec2 zoom = vec2(offset, offset);
    _eye = _center + (_eye - _center) * offset;
    zoom *= offset;
}

void Camera::rotate(int xrel, int yrel)
{
    _eye -= _center;
    _up -= _center;
    float xangle = (xrel / resolution.x) * tau;
    _eye = glm::rotate(_eye, -xangle, _up);
    float yangle = (yrel / resolution.y) * tau;
    vec3 haxis = cross(_eye - _center, _up);
    _eye = glm::rotate(_eye, -yangle, haxis);
    _up = glm::rotate(_up, -yangle, haxis);
    _eye += _center;
    _up += _center;
}

void Camera::move(float xrel, float yrel)
{
    xrel *= zoom;
    yrel *= zoom;
    
    float distance_3d = length(_eye - _center);
    vec3 voff = normalize(_up) * (yrel / resolution.y) * distance_3d * camera_speed;
    _center -= voff;
    _eye -= voff;
    vec3 haxis = cross(_eye - _center, _up);
    vec3 hoff = normalize(haxis) * (xrel / resolution.x) * distance_3d  * camera_speed;
    _center += hoff;
    _eye += hoff;
}

float *Camera::getScrollFactor()
{
    return &scroll_factor;
}

float *Camera::getCameraSpeed()
{
    return &camera_speed;
}