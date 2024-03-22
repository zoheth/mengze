#pragma once

#include <memory>
#include <array>
#include <optional>

#include <glm/glm.hpp>

#include "ray_tracing/aabb.h"
#include "ray_tracing/hittable.h"

namespace mengze::rt
{
class Triangle : public Hittable
{
  public:
	Triangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, const std::shared_ptr<Material> &material, const std::optional<std::array<glm::vec2, 3>> &uv);

	bool hit(const Ray &r, Interval ray_t, HitRecord &rec) const override;

	float pdf_value(const glm::vec3 &origin, const glm::vec3 &direction) const override;


	glm::vec3 random(const glm::vec3 &origin) const override;

	Aabb bounding_box() const override;

  private:
	void set_bounding_box();

private:
	glm::vec3 v0_;
	glm::vec3 v1_;
	glm::vec3 v2_;
	glm::vec3 normal_;
	std::optional < std::array<glm::vec2, 3>> uv_;
	float     area_;

	Aabb                      b_box_;
	std::shared_ptr<Material> material_;
};

}        // namespace mengze::rt
