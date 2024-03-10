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

CosinePdf::CosinePdf(const glm::vec3 &w)
{
	uvw_.build_from_w(w);
}

float CosinePdf::value(const glm::vec3 &direction) const
{
	auto cosine = glm::dot(glm::normalize(direction), uvw_.w());
	return (cosine <= 0) ? 0 : cosine / glm::pi<float>();
}

glm::vec3 CosinePdf::generate() const
{
	return uvw_.to_local(random_cosine_direction());
}

MixturePdf::MixturePdf(std::shared_ptr<Pdf> p0, std::shared_ptr<Pdf> p1) :
    p0_(std::move(p0)), p1_(std::move(p1))
{}

float MixturePdf::value(const glm::vec3 &direction) const
{
	return 0.5f * p0_->value(direction) + 0.5f * p1_->value(direction);
}

glm::vec3 MixturePdf::generate() const
{
	if (random_float() < 0.5f)
		return p0_->generate();
	else
		return p1_->generate();
}
}        // namespace mengze::rt
