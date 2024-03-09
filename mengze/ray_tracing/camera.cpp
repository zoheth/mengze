#include "camera.h"

namespace mengze::rt
{
Camera::Camera(glm::vec3 position, glm::vec3 forward, float fov) :
    mengze::Camera(position, forward, fov)
{
}

Camera::Camera(glm::vec3 position, glm::vec3 look_at, glm::vec3 up, float fov) :
	mengze::Camera(position, look_at, up, fov)
{}
}        // namespace mengze::rt
