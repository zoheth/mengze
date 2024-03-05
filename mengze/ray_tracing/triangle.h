#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "aabb.h"
#include "hittable.h"

namespace mengze::rt
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

class Quad : public Hittable
{
  public:
	Quad(const glm::vec3 &Q, const glm::vec3 &u, const glm::vec3 &v, const std::shared_ptr<Material> &material) :
	    Q_(Q), u_(u), v_(v), material_(material)
	{
		auto n = glm::cross(u_, v_);

		normal_ = glm::normalize(n);
		D       = glm::dot(normal_, Q_);
		w       = n / glm::dot(n,n);
	}

	virtual void set_bounding_box()
	{
		b_box_ = Aabb(Q_, Q_ + u_ + v_).pad();
	}

	bool hit(const Ray &r, Interval ray_t, HitRecord &rec) const override
	{
		float denom = glm::dot(normal_, r.direction());

		if (std::fabs(denom) < 1e-8f)
			return false;

		auto t = (D - glm::dot(normal_, r.origin())) / denom;
		if (!ray_t.contains(t))
			return false;

		auto      intersection        = r.at(t);
		glm::vec3 planar_hitpt_vector = intersection - Q_;

		auto alpha = glm::dot(w, glm::cross(planar_hitpt_vector, v_));
		auto beta  = glm::dot(w, glm::cross(u_, planar_hitpt_vector));

		if (!is_interior(alpha, beta, rec))
			return false;

		rec.t        = t;
		rec.position = intersection;
		rec.material = material_;
		rec.set_face_normal(r, normal_);

		return true;
	}

	bool is_interior(float a, float b, HitRecord &rec) const
	{
		if ((a < 0) || (1 < a) || (b < 0) || (1 < b))
			return false;
		rec.u = a;
		rec.v = b;
		return true;
	}

  private:
	glm::vec3 Q_;
	glm::vec3 u_;
	glm::vec3 v_;
	glm::vec3 normal_;
	Aabb      b_box_;

	std::shared_ptr<Material> material_;

	float     D;
	glm::vec3 w;
};
}        // namespace mengze::rt
