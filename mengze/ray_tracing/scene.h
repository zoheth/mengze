#pragma once
#include <memory>
#include <vector>

#include "hittable.h"

namespace mengze::rt
{

class HittableList : public Hittable
{
  public:
	void add(const std::shared_ptr<Hittable> &object);

	bool hit(const Ray &r, Interval ray_t, HitRecord &rec) const override;

	float pdf_value(const glm::vec3 &origin, const glm::vec3 &direction) const override;

	glm::vec3 random(const glm::vec3 &origin) const override;

  private:
	std::vector<std::shared_ptr<Hittable>> objects_;
};

class Scene
{
  public:
	void add(const std::shared_ptr<Hittable> &object);

	void add_light(const std::shared_ptr<Hittable> &light);

	HittableList &world()
	{
		return world_;
	}
	HittableList &lights()
	{
		return lights_;
	}

  private:
	HittableList world_;
	HittableList lights_;
};
}        // namespace mengze::rt
