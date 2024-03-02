#pragma once

#include <glm/glm.hpp>

#include "util/math.h"
#include "ray.h"
#include "material.h"

namespace mengze
{

struct HitRecord
{
	glm::vec3 position;
	glm::vec3 normal;
	std::shared_ptr<Material> material;
	float     t;
	bool      front_face;

	inline void set_face_normal(const Ray &r, const glm::vec3 &outward_normal)
	{
		front_face = glm::dot(r.direction(), outward_normal) < 0;
		normal     = front_face ? outward_normal : -outward_normal;
	}
};

class Hittable
{
  public:
	Hittable()          = default;
	virtual ~Hittable() = default;

	Hittable(const Hittable &)            = delete;
	Hittable &operator=(const Hittable &) = delete;
	// Move constructor don't need to be deleted
	Hittable(Hittable &&)            = default;
	Hittable &operator=(Hittable &&) = delete;

	virtual bool hit(const Ray &r, Interval ray_t, HitRecord &rec) const = 0;
};
}        // namespace mengze
