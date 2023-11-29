#pragma once

#include <memory>

#include "camera.h"
#include "core/image.h"

namespace mengze
{
	class Renderer
	{
	public:
		Renderer() = default;
		virtual ~Renderer() = default;

		virtual void on_update(float ts) {}

		virtual void on_resize(uint32_t width, uint32_t height);
		virtual void render() = 0;

		void set_pixel(uint32_t x, uint32_t y, const glm::vec3& color);

		void present() { film_->set_data(film_data_); }

		uint32_t get_width() const { return film_->get_width(); }
		uint32_t get_height() const { return film_->get_height(); }

		std::shared_ptr<Image> get_film() const { return film_; }

	private:
		std::shared_ptr<Image> film_;
		// 32 bit RGBA
		uint32_t* film_data_ = nullptr;
	};
}