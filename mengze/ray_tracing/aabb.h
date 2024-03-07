#pragma once

#include "ray_tracing/math.h"

namespace mengze::rt
{
class Aabb
{
  public:
	Aabb() = default;

	Aabb(const Interval &x, const Interval &y, const Interval &z) :
	    x_(x), y_(y), z_(z)
	{}

	Aabb(const glm::vec3 &min, const glm::vec3 &max) :
	    x_(min.x, max.x), y_(min.y, max.y), z_(min.z, max.z)
	{}

	Aabb pad()
	{
		float    delta = 0.0001;
		Interval x     = (x_.size() > delta) ? x_ : x_.expand(delta);
		Interval y     = (y_.size() > delta) ? y_ : y_.expand(delta);
		Interval z     = (z_.size() > delta) ? z_ : z_.expand(delta);
		return Aabb(x, y, z);
	}

  private:
	Interval x_, y_, z_;
};
}        // namespace mengze
