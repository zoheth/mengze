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

		void clear(const glm::vec3& color);

		void set_pixel(uint32_t x, uint32_t y, const glm::vec3& color);

		void present() { final_image_->set_data(image_data_); }

		uint32_t get_width() const { return final_image_->get_width(); }
		uint32_t get_height() const { return final_image_->get_height(); }

		std::shared_ptr<Image> get_final_image() const { return final_image_; }

	protected:
		glm::vec3 &get_pixel_accumulation(uint32_t x, uint32_t y) const;
		void reset_accumulation() const;

	    glm::vec3 *accumulation_data_ = nullptr;

		bool is_accumulation_ = false;

		uint32_t frame_index_ = 1;

		std::vector<uint32_t> image_horizontal_iter_, image_vertical_iter_;
	private:
		std::shared_ptr<Image> final_image_;
		// 32 bit RGBA
		uint32_t* image_data_ = nullptr;

	};
}