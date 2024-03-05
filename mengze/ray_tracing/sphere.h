#pragma once

#include <glm/glm.hpp>

#include "hittable.h"
#include "material.h"

namespace mengze::rt
{

class Sphere : public Hittable
{
  public:
	Sphere(glm::vec3 center, float radius, std::shared_ptr<Material> material) :
	    center_(center), radius_(radius), material_(material)
	{}

	bool hit(const Ray &r, Interval ray_t, HitRecord &rec) const override
	{
		glm::vec3 oc           = r.origin() - center_;
		float     a            = glm::dot(r.direction(), r.direction());
		float     half_b       = glm::dot(oc, r.direction());
		float     c            = glm::dot(oc, oc) - radius_ * radius_;
		float     discriminant = half_b * half_b - a * c;

		if (discriminant < 0)
			return false;

		auto sqrt_d = glm::sqrt(discriminant);
		auto root   = (-half_b - sqrt_d) / a;
		if (!ray_t.surrounds(root))
		{
			root = (-half_b + sqrt_d) / a;
			if (!ray_t.surrounds(root))
				return false;
		}

		rec.t                    = root;
		rec.position             = r.at(rec.t);
		glm::vec3 outward_normal = (rec.position - center_) / radius_;
		rec.set_face_normal(r, outward_normal);
		rec.material = material_;

		return true;
	}

  private:
	glm::vec3                 center_;
	float                     radius_{0.f};
	std::shared_ptr<Material> material_;
};
}        // namespace mengze
