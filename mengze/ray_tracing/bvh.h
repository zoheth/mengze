#pragma once

#include "ray_tracing/aabb.h"
#include "ray_tracing/hittable.h"

namespace mengze::rt
{
class HittableList;
class BvhNode final : public Hittable
{
  public:
	BvhNode(const HittableList &list);

	BvhNode(const std::vector<std::shared_ptr<Hittable>> &objects, size_t start, size_t end, bool is_root = true, int depth = 0);

	bool hit(const Ray &r, Interval ray_t, HitRecord &rec) const override;

	Aabb bounding_box() const override;

	float pdf_value(const glm::vec3 &origin, const glm::vec3 &direction) const override;

	glm::vec3 random(const glm::vec3 &origin) const override;

private:
	static bool box_compare(const std::shared_ptr<Hittable> &a, const std::shared_ptr<Hittable> &b, int axis);
	static bool box_x_compare(const std::shared_ptr<Hittable> &a, const std::shared_ptr<Hittable> &b);
	static bool box_y_compare(const std::shared_ptr<Hittable> &a, const std::shared_ptr<Hittable> &b);
	static bool box_z_compare(const std::shared_ptr<Hittable> &a, const std::shared_ptr<Hittable> &b);

  private:
	std::shared_ptr<Hittable> left_;
	std::shared_ptr<Hittable> right_;

	bool                                   is_root_;
	std::vector<std::shared_ptr<Hittable>> root_all_objects_; // only used for root node

	Aabb box_;
};
}        // namespace mengze