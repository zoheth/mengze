#pragma once
#include <string>

#include <glm/glm.hpp>

#include "core/logging.h"

namespace mengze::rt
{

class Image
{
  public:
	Image(const std::string &filename)
	{
		load(filename);
	}

	bool load(const std::string &filename);

	int width() const;

	int height() const;

	const uint8_t *pixel_data(int x, int y) const;

  private:
	int      bytes_per_pixel_{3};
	uint8_t *data_{nullptr};
	int      width_{0};
	int      height_{0};
	int      bytes_per_scanline_{0};

	static int clamp(int x, int min, int max);
};

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

class ImageTexture : public Texture
{
  public:
	ImageTexture(const std::string &filename) :
	    image_(filename)
	{
	}

	glm::vec3 value(float u, float v, const glm::vec3 &p) const override;

  private:
	Image image_;
};

}        // namespace mengze::rt
