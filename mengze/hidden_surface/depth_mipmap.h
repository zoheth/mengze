#pragma once

#include <vector>


namespace mengze
{
	class DepthMipmap {
	public:
		DepthMipmap(const uint32_t width, const uint32_t height);

		void update_depth(uint32_t x, uint32_t y, float depth);

		//void update_higher_levels_depth(uint32_t min_x, uint32_t max_x, uint32_t min_y, uint32_t max_y);

		[[nodiscard]] bool is_occluded(uint32_t min_x, uint32_t max_x, uint32_t min_y, uint32_t max_y, float depth) const;

		void reset();

		[[nodiscard]] float get_depth(uint32_t x, uint32_t y) const;

		void set_depth(uint32_t x, uint32_t y, float depth);

	private:
		[[nodiscard]] uint32_t get_index(uint32_t x, uint32_t y, uint32_t level) const;

	private:
		uint32_t width_, height_, levels_;
		std::vector<std::vector<float>> depth_buffers_;
	};
}