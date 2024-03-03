#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "hittable.h"

namespace mengze
{
class Triangle : public Hittable
{
  public:
	Triangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, const std::shared_ptr<Material> &material) :
	    v0_(v0), v1_(v1), v2_(v2), material_(material)
	{
		normal_ = glm::normalize(glm::cross(v1_ - v0_, v2_ - v0_));
		set_bounding_box();
	}

	bool hit(const Ray &r, Interval ray_t, HitRecord &rec) const override
	{
		constexpr float epsilon = 1e-8f;

		glm::vec3 edge1 = v1_ - v0_;
		glm::vec3 edge2 = v2_ - v0_;
		glm::vec3 h     = glm::cross(r.direction(), edge2);
		float     a     = glm::dot(edge1, h);

		if (a > -epsilon && a < epsilon)
			return false;

		float     f = 1.0f / a;
		glm::vec3 s = r.origin() - v0_;
		float     u = f * glm::dot(s, h);

		if (u < 0.0f || u > 1.0f)
			return false;

		glm::vec3 q = glm::cross(s, edge1);
		float     v = f * glm::dot(r.direction(), q);

		if (v < 0.0f || u + v > 1.0f)
			return false;

		float t = f * glm::dot(edge2, q);

		if (t < ray_t.min() || t > ray_t.max())
			return false;

		rec.t        = t;
		rec.position = r.at(t);
		rec.material = material_;
		rec.set_face_normal(r, normal_);
		return true;
	}

private:
	void set_bounding_box()
	{
		glm::vec3 min_v = glm::min(v0_, glm::min(v1_, v2_));

		glm::vec3 max_v = glm::max(v0_, glm::max(v1_, v2_));

		b_box_ = Aabb(min_v, max_v);
	}

  private:
	glm::vec3                 v0_;
	glm::vec3                 v1_;
	glm::vec3                 v2_;
	glm::vec3                 normal_;
	Aabb                      b_box_;
	std::shared_ptr<Material> material_;
};
}        // namespace mengze
