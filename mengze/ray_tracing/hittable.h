#pragma once

#include <glm/glm.hpp>

#include "material.h"
#include "ray.h"
#include "ray_tracing/math.h"

namespace mengze::rt
{

struct HitRecord
{
	glm::vec3                 position;
	glm::vec3                 normal;
	std::shared_ptr<Material> material;

	float t;
	float u;
	float v;

	bool front_face;

	void set_face_normal(const Ray &r, const glm::vec3 &outward_normal);
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

	virtual float pdf_value(const glm::vec3 &origin, const glm::vec3 &direction) const
	{
		return 0.0f;
	}

	virtual glm::vec3 random(const glm::vec3 &origin) const
	{
		return {1.0f, 0.0f, 0.0f};
	}
};

class HittablePdf : public Pdf
{
  public:
	HittablePdf(Hittable &p, const glm::vec3 &origin) :
	    p_(p), origin_(origin)
	{}

	float value(const glm::vec3 &direction) const override
	{
		return p_.pdf_value(origin_, direction);
	}

	glm::vec3 generate() const override
	{
		return p_.random(origin_);
	}

  private:
	Hittable &p_;
	glm::vec3 origin_;
};
}        // namespace mengze::rt
