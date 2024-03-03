#pragma once

#include "util/math.h"

namespace mengze
{
class Aabb
{
  public:
	Aabb(const glm::vec3 &min, const glm::vec3 &max) : x_(min.x, max.x), y_(min.y, max.y), z_(min.z, max.z) {}
	
  private:
	Interval x_, y_, z_;
};
}        // namespace mengze
