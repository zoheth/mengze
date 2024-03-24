#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "ray_tracing/math.h"

namespace mengze
{
bool rt::Image::load(const std::string &filename)
{
	auto components = bytes_per_pixel_;
	data_           = stbi_load(filename.c_str(), &width_, &height_, &components, components);
	if (!data_)
	{
		LOGE("Failed to load image: %s", filename.c_str())
		width_ = height_    = 0;
		bytes_per_scanline_ = 0;
		return false;
	}
	bytes_per_scanline_ = bytes_per_pixel_ * width_;
	return true;
}

int rt::Image::width() const
{
	return width_;
}

int rt::Image::height() const
{
	return height_;
}

const uint8_t *rt::Image::pixel_data(int x, int y) const
{
	static uint8_t magenta[3] = {255, 0, 255};
	if (!data_)
	{
		return magenta;
	}

	x = clamp(x, 0, width_);
	y = clamp(y, 0, height_);
	return data_ + y * bytes_per_scanline_ + x * bytes_per_pixel_;
}

int rt::Image::clamp(int x, int min, int max)
{
	if (x < min)
	{
		return min;
	}
	if (x < max)
	{
		return x;
	}
	return max - 1;
}

glm::vec3 rt::ImageTexture::value(float u, float v, const glm::vec3 &p) const
{
	if (image_.width() == 0 || image_.height() == 0)
	{
		return glm::vec3(0.0f, 0.1f, 0.1f);
	}

	u = fmod(u, 1.0f);
	if (u < 0)
		u += 1.0f;
	v = fmod(v, 1.0f);
	if (v < 0)
		v += 1.0f;

	auto i = static_cast<int>(u * image_.width()) % image_.width();
	auto j = static_cast<int>((1.0f - v) * image_.height()) % image_.height();

	auto pixel = image_.pixel_data(i, j);

	auto color_scale = 1.0f / 255.0f;
	return color_scale * glm::vec3(pixel[0], pixel[1], pixel[2]);
}
}        // namespace mengze
