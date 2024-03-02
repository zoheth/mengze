#include "renderer.h"

namespace 
{
	uint32_t to_rgba(const glm::vec3& color)
	{
		uint32_t r = static_cast<uint32_t>(color.r * 255.0f);
		uint32_t g = static_cast<uint32_t>(color.g * 255.0f);
		uint32_t b = static_cast<uint32_t>(color.b * 255.0f);
		uint32_t a = 0xff;
		return (a << 24) | (b << 16) | (g << 8) | r;
	}
}

namespace mengze
{
	void Renderer::on_resize(uint32_t width, uint32_t height)
	{
		if (film_)
		{
			if (film_->get_width() == width && film_->get_height() == height)
			{
				return;
			}
			// film_->resize(width, height);
		}
		film_ = std::make_shared<Image>(width, height, ImageFormat::kR8G8B8A8);
		

		delete[] film_data_;
		film_data_ = new uint32_t[width * height];
	}

	void Renderer::clear(const glm::vec3& color)
	{
		uint32_t rgba = to_rgba(color);
		for (uint32_t i = 0; i < film_->get_width() * film_->get_height(); ++i)
		{
			film_data_[i] = rgba;
		}
	}

	void Renderer::set_pixel(uint32_t x, uint32_t y, const glm::vec3& color)
	{
		//film_data_[y * film_->get_width() + x] = to_rgba(glm::clamp(color, 0.0f, 1.0f));

		// linear to gamma
		glm::vec3 write_color = glm::pow(color, glm::vec3(1.0f / 2.2f));

		film_data_[(film_->get_height() - 1 - y) * film_->get_width() + x] = to_rgba(glm::clamp(write_color, 0.0f, 1.0f));
	}
}
