#pragma once
#include "glm/ext/scalar_constants.hpp"

#include <algorithm>
#include <limits>
#include <cmath>
#include <random>

#ifndef M_PI
#	define M_PI 3.14159265358979323846
#endif

namespace mengze
{
template <template <typename> class Child, typename T>
class Tuple3
{
  public:
	Tuple3() = default;

	Tuple3(T x, T y, T z) :
	    x(x), y(y), z(z)
	{}

	T x{}, y{}, z{};
};

template <typename T>
class Vector3 : public Tuple3<Vector3, T>
{
  public:
	using Tuple3<Vector3, T>::x;
	using Tuple3<Vector3, T>::y;
	using Tuple3<Vector3, T>::z;

	Vector3() = default;

	Vector3(T x, T y, T z) :
	    Tuple3<Vector3, T>(x, y, z)
	{}
};

template <typename T>
class Point3 : public Tuple3<Point3, T>
{
  public:
	using Tuple3<Point3, T>::x;
	using Tuple3<Point3, T>::y;
	using Tuple3<Point3, T>::z;

	Point3() = default;

	Point3(T x, T y, T z) :
	    Tuple3<Point3, T>(x, y, z)
	{}
};

class Interval
{
  public:
	Interval() :
	    min_(std::numeric_limits<float>::infinity()),
	    max_(-std::numeric_limits<float>::infinity())
	{}

	Interval(float min, float max = std::numeric_limits<float>::infinity()) :
	    min_(min), max_(max)
	{}

	float min() const
	{
		return min_;
	}
	float max() const
	{
		return max_;
	}

	// Computes the size of the interval.
	float size() const
	{
		return max_ - min_;
	}

	// Expands the interval by delta, distributed evenly to both sides.
	Interval expand(float delta) const
	{
		return Interval(min_ - delta / 2, max_ + delta / 2);
	}

	// Checks if the interval contains the value x.
	bool contains(float x) const
	{
		return x >= min_ && x <= max_;
	}

	// Checks if the interval strictly surrounds the value x.
	bool surrounds(float x) const
	{
		return x > min_ && x < max_;
	}

	float clamp(float x) const
	{
		return std::max(min_, std::min(x, max_));
	}

  private:
	float min_, max_;
};

// Utility function to generate a random double in [min,max).
inline float random_float(const float min = 0.0f, const float max = 1.0f)
{
	static std::random_device                      rd;
	std::uniform_real_distribution<float> distribution(min, max);
	static std::mt19937                             generator(rd());
	return distribution(generator);
}

// Generates a random unit vector.
inline glm::vec3 random_unit_vector()
{
	auto a = random_float(0, glm::pi<float>());             // Random azimuthal angle
	auto z = random_float(-1, 1);          // Random cosine of the zenith angle
	auto r = sqrt(1 - z * z);                   // Radius in the xy-plane

	return {r * cos(a), r * sin(a), z};
}

inline glm::vec3 random_vec3(const float min = 0.0f, const float max = 1.0f)
{
	return {random_float(min, max), random_float(min, max), random_float(min, max)};
}

inline glm::vec3 random_cosine_direction()
{
	float r1 = random_float();
	float r2 = random_float();
	float z  = sqrt(1 - r2);

	float phi = 2 * M_PI * r1;
	float x   = cos(phi) * sqrt(r2);
	float y   = sin(phi) * sqrt(r2);

	return glm::vec3(x, y, z);
}

inline bool near_zero(const glm::vec3 &v, float epsilon = 1e-8f)
{
	// Check if the vector is close to zero in all dimensions.
	return glm::abs(v.x) < epsilon && glm::abs(v.y) < epsilon && glm::abs(v.z) < epsilon;
}

}        // namespace mengze