#pragma once

#include "aabb.h"

namespace mengze
{
class BvhNode : public Hittable
{
  public:
	BvhNode() = default;

  private:
	std::shared_ptr<Hittable> left_;
	std::shared_ptr<Hittable> right_;

	Aabb box_;
};
}        // namespace mengze