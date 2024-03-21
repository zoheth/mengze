#pragma once
#include <algorithm>
#include <cmath>
#include <limits>
#include <random>

#include <glm/ext/scalar_constants.hpp>
#include <glm/glm.hpp>

#ifndef M_PI
#	define M_PI 3.14159265358979323846
#endif

namespace mengze::rt
{
class Interval
{
  public:
	Interval();

	Interval(float min, float max = std::numeric_limits<float>::infinity());

	Interval(const Interval &a, const Interval &b) :
		min_(std::min(a.min_, b.min_)),
		max_(std::max(a.max_, b.max_))
	{}

	float min() const;

	float max() const;

	float &min();

	float &max();

	float size() const;

	Interval expand(float delta) const;

	bool contains(float x) const;

	bool surrounds(float x) const;

	float clamp(float x) const;

  private:
	float min_, max_;
};

class OrthoNormalBasis
{
  public:
	OrthoNormalBasis() = default;

	glm::vec3 operator[](int i) const
	{
		return axis_[i];
	}
	glm::vec3 &operator[](int i)
	{
		return axis_[i];
	}

	glm::vec3 u() const
	{
		return axis_[0];
	}
	glm::vec3 v() const
	{
		return axis_[1];
	}
	glm::vec3 w() const
	{
		return axis_[2];
	}

	glm::vec3 to_local(float a, float b, float c) const
	{
		return a * u() + b * v() + c * w();
	}

	glm::vec3 to_local(const glm::vec3 &a) const
	{
		return a.x * u() + a.y * v() + a.z * w();
	}

	void build_from_w(const glm::vec3 &w)
	{
		glm::vec3 unitW = glm::normalize(w);
		glm::vec3 a     = (fabs(unitW.x) > 0.9f) ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
		glm::vec3 v     = glm::normalize(glm::cross(unitW, a));
		glm::vec3 u     = glm::cross(unitW, v);

		axis_[0] = u;
		axis_[1] = v;
		axis_[2] = unitW;
	}

  private:
	glm::vec3 axis_[3];
};


inline float random_float(const float min = 0.0f, const float max = 1.0f)
{
	static std::random_device             rd;
	std::uniform_real_distribution<float> distribution(min, max);
	static std::mt19937                   generator(rd());
	return distribution(generator);
}

inline glm::vec3 random_unit_vector()
{
	auto a = random_float(0, glm::pi<float>());        // Random azimuthal angle
	auto z = random_float(-1, 1);                      // Random cosine of the zenith angle
	auto r = sqrt(1 - z * z);                          // Radius in the xy-plane

	return {r * cos(a), r * sin(a), z};
}

inline glm::vec3 random_vec3(const float min = 0.0f, const float max = 1.0f)
{
	return {random_float(min, max), random_float(min, max), random_float(min, max)};
}

inline glm::vec3 random_in_unit_sphere()
{
	while (true)
	{
		auto p = random_vec3(-1, 1);
		if (glm::dot(p, p) < 1)
			return p;
	}
}

inline glm::vec3 random_cosine_direction()
{
	float r1 = random_float();
	float r2 = random_float();
	float z  = sqrt(1 - r2);

	float phi = 2 * glm::pi<float>() * r1;
	float x   = cos(phi) * sqrt(r2);
	float y   = sin(phi) * sqrt(r2);

	return {x, y, z};
}

inline bool near_zero(const glm::vec3 &v, float epsilon = 1e-8f)
{
	return glm::abs(v.x) < epsilon && glm::abs(v.y) < epsilon && glm::abs(v.z) < epsilon;
}
}        // namespace mengze::rt
