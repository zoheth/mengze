#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "ray_tracing/aabb.h"
#include "ray_tracing/hittable.h"

namespace mengze::rt
{
class Triangle : public Hittable
{
  public:
	Triangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, const std::shared_ptr<Material> &material);

	bool hit(const Ray &r, Interval ray_t, HitRecord &rec) const override;

	float pdf_value(const glm::vec3 &origin, const glm::vec3 &direction) const override;


	glm::vec3 random(const glm::vec3 &origin) const override;

  private:
	void set_bounding_box()
	{
		glm::vec3 min_v = glm::min(v0_, glm::min(v1_, v2_));

		glm::vec3 max_v = glm::max(v0_, glm::max(v1_, v2_));

		b_box_ = Aabb(min_v, max_v);
	}

  private:
	glm::vec3 v0_;
	glm::vec3 v1_;
	glm::vec3 v2_;
	glm::vec3 normal_;
	float     area_;

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
		w       = n / glm::dot(n, n);
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
