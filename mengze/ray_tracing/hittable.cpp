#include "hittable.h"

namespace mengze::rt
{
void HitRecord::set_face_normal(const Ray &r, const glm::vec3 &outward_normal)
{
	front_face = glm::dot(r.direction(), outward_normal) < 0;
	normal     = front_face ? outward_normal : -outward_normal;
}
}
