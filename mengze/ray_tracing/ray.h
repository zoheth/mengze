#pragma once

#include "glm/glm.hpp"

namespace mengze
{
class Ray
{
  public:
	Ray() = default;

	Ray(const glm::vec3 &origin, const glm::vec3 &direction) :
	    origin_{origin},
	    direction_{direction}
	{
	}

	const glm::vec3 &get_origin() const
	{
		return origin_;
	}
	const glm::vec3 &get_direction() const
	{
		return direction_;
	}

  private:
	glm::vec3 origin_;
	glm::vec3 direction_;
};
}        // namespace mengze
