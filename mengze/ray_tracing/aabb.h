#pragma once

#include "ray_tracing/math.h"
#include "ray_tracing/ray.h"

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

	Aabb(const Aabb &a, const Aabb &b) :
	    x_(Interval(a.x_, b.x_)), y_(Interval(a.y_, b.y_)), z_(Interval(a.z_, b.z_))
	{}

	Aabb pad()
	{
		float    delta = 0.0001;
		Interval x     = (x_.size() > delta) ? x_ : x_.expand(delta);
		Interval y     = (y_.size() > delta) ? y_ : y_.expand(delta);
		Interval z     = (z_.size() > delta) ? z_ : z_.expand(delta);
		return Aabb(x, y, z);
	}

	const Interval &axis(int n) const
	{
		if (n == 1)
			return y_;
		if (n == 2)
			return z_;
		return x_;
	}

	bool hit(const Ray &r, Interval ray_t) const
	{
		for (int i = 0; i < 3; i++)
		{
			auto inv_d  = 1.0f / r.direction()[i];
			auto origin = r.origin()[i];

			auto t0 = (axis(i).min() - origin) * inv_d;
			auto t1 = (axis(i).max() - origin) * inv_d;

			if (inv_d < 0.0f)
			{
				std::swap(t0, t1);
			}

			if (t0 > ray_t.min())
			{
				ray_t.min() = t0;
			}
			if (t1 < ray_t.max())
			{
				ray_t.max() = t1;
			}

			if (ray_t.max() <= ray_t.min())
			{
				return false;
			}
		}
		return true;
	}

  private:
	Interval x_, y_, z_;
};

}        // namespace mengze::rt
