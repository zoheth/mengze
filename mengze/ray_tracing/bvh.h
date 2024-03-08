#pragma once

#include "ray_tracing/aabb.h"
#include "ray_tracing/hittable.h"

namespace mengze::rt
{
class BvhNode : public Hittable
{
  public:
	BvhNode() = default;

private:
	static bool box_compare(const std::shared_ptr<Hittable> &a, const std::shared_ptr<Hittable> &b, int axis);

  private:
	std::shared_ptr<Hittable> left_;
	std::shared_ptr<Hittable> right_;

	Aabb box_;
};
}        // namespace mengze