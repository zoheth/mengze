#include "ray_tracing/math.h"

namespace mengze::rt
{
Interval::Interval() :
    min_(std::numeric_limits<float>::infinity()),
    max_(-std::numeric_limits<float>::infinity())
{}

Interval::Interval(float min, float max) :
    min_(min), max_(max)
{}

float Interval::min() const
{
	return min_;
}

float Interval::max() const
{
	return max_;
}

float & Interval::min()
{
	return min_;
}

float & Interval::max()
{
	return max_;
}


float Interval::size() const
{
	return max_ - min_;
}

Interval Interval::expand(float delta) const
{
	return {min_ - delta / 2, max_ + delta / 2};
}

bool Interval::contains(float x) const
{
	return x >= min_ && x <= max_;
}

bool Interval::surrounds(float x) const
{
	return x > min_ && x < max_;
}

float Interval::clamp(float x) const
{
	return std::max(min_, std::min(x, max_));
}
}        // namespace mengze::rt
