#include "renderer.h"

namespace
{
uint32_t to_rgba(const glm::vec3 &color)
{
	uint32_t r = static_cast<uint32_t>(color.r * 255.0f);
	uint32_t g = static_cast<uint32_t>(color.g * 255.0f);
	uint32_t b = static_cast<uint32_t>(color.b * 255.0f);
	uint32_t a = 0xff;
	return (a << 24) | (b << 16) | (g << 8) | r;
}
}        // namespace

namespace mengze
{
void Renderer::on_resize(uint32_t width, uint32_t height)
{
	if (final_image_)
	{
		if (final_image_->get_width() == width && final_image_->get_height() == height)
		{
			return;
		}
		// film_->resize(width, height);
	}

	frame_index_ = 1;

	final_image_ = std::make_shared<Image>(width, height, ImageFormat::kR8G8B8A8);

	delete[] image_data_;
	image_data_ = new uint32_t[width * height];

	delete[] accumulation_data_;
	accumulation_data_ = new glm::vec3[width * height];

	image_horizontal_iter_.resize(width);
	image_vertical_iter_.resize(height);
	for (uint32_t i = 0; i < width; ++i)
	{
		image_horizontal_iter_[i] = i;
	}
	for (uint32_t i = 0; i < height; ++i)
	{
		image_vertical_iter_[i] = i;
	}
}

void Renderer::clear(const glm::vec3 &color)
{
	uint32_t rgba = to_rgba(color);
	for (uint32_t i = 0; i < final_image_->get_width() * final_image_->get_height(); ++i)
	{
		image_data_[i] = rgba;
	}
}

void Renderer::set_pixel(uint32_t x, uint32_t y, const glm::vec3 &color)
{
	// film_data_[y * film_->get_width() + x] = to_rgba(glm::clamp(color, 0.0f, 1.0f));

	// linear to gamma
	glm::vec3 write_color = glm::pow(color, glm::vec3(1.0f / 2.2f));

	image_data_[(final_image_->get_height() - 1 - y) * final_image_->get_width() + x] = to_rgba(glm::clamp(write_color, 0.0f, 1.0f));
}

glm::vec3 &Renderer::get_pixel_accumulation(uint32_t x, uint32_t y) const
{
	return accumulation_data_[y * final_image_->get_width() + x];
}

void Renderer::reset_accumulation() const
{
	memset(accumulation_data_, 0, final_image_->get_width() * final_image_->get_height() * sizeof(glm::vec3));
}
}        // namespace mengze
