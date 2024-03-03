#pragma once
#include <glm/glm.hpp>

namespace mengze
{
class Texture
{
  public:
	virtual ~Texture() = default;

	virtual glm::vec3 value(float u, float v, const glm::vec3 &p) const = 0;
};

class SolidColor : public Texture
{
  public:
	SolidColor(glm::vec3 c) :
		color_value_(c)
	{}

	SolidColor(float red, float green, float blue) :
	    SolidColor(glm::vec3(red, green, blue))
	{}

	virtual glm::vec3 value(float u, float v, const glm::vec3 &p) const override
	{
		return color_value_;
	}

  private:
	glm::vec3 color_value_;
};

}        // namespace mengze
