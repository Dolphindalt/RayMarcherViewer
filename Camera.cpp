#include "Camera.h"

#include <SDL2/SDL.h>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/scalar_multiplication.hpp>

using namespace glm;

extern glm::vec3 resolution;
extern int last_x, last_y;

const float camera_speed = 0.05f;
const float zoom_factor = 1.1892f;
const float scroll_factor = 0.1;
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
    offset = (offset > 0) ? 1.0 + scroll_factor : -1.0 - scroll_factor;
    float z = pow(zoom_factor, offset);
    vec2 zoom = vec2(z, z);
    _eye = _center + (_eye - _center) * z;
    zoom *= z;
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
    vec3 voff = normalize(_up) * (yrel / resolution.y) * distance_3d;
    _center -= voff;
    _eye -= voff;
    vec3 haxis = cross(_eye - _center, _up);
    vec3 hoff = normalize(haxis) * (xrel / resolution.x) * distance_3d;
    _center += hoff;
    _eye += hoff;
}

void Camera::w()
{
    
}

void Camera::a()
{
    
}

void Camera::s()
{
    
}

void Camera::d()
{
    
}