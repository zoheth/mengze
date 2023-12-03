#include "hierarchical_struct.h"

#include <algorithm>
#include <cmath>

namespace mengze
{
	MultiLevelDepthBuffer::MultiLevelDepthBuffer(const uint32_t width, const uint32_t height) : width_(width), height_(height)
	{
		levels_ = std::floor(std::log2(std::min(width, height)));

		for (uint32_t level = 0; level < levels_; ++level) {
			const uint32_t level_width = (width >> level) + 1;
			const uint32_t level_height = (height >> level) + 1;
			depth_buffers_.emplace_back(level_width * level_height, std::numeric_limits<float>::infinity());
		}
	}
	//void MultiLevelDepthBuffer::update_higher_levels_depth(uint32_t min_x, uint32_t max_x, uint32_t min_y, uint32_t max_y) {

	//	for (uint32_t level = 1; level < levels_; ++level) {
	//		uint32_t level_min_x = min_x >> level;
	//		uint32_t level_max_x = (max_x >> level) + 1; 
	//		uint32_t level_min_y = min_y >> level;
	//		uint32_t level_max_y = (max_y >> level) + 1;
	//		uint32_t level_width = width_ >> level;

	//		for (uint32_t y = level_min_y; y <= level_max_y; ++y) {
	//			for (uint32_t x = level_min_x; x <= level_max_x; ++x) {
	//				if (x >= level_width || y >= (height_ >> level)) {
	//					continue;
	//				}

	//				uint32_t index = y * level_width + x;
	//				float max_depth = -std::numeric_limits<float>::infinity();

	//				for (int dy = 0; dy < 2; ++dy) {
	//					for (int dx = 0; dx < 2; ++dx) {
	//						uint32_t lower_x = (x << 1) + dx;
	//						uint32_t lower_y = (y << 1) + dy;
	//						if (lower_x < (width_ >> (level - 1)) && lower_y < (height_ >> (level - 1))) {
	//							uint32_t lower_index = lower_y * (level_width << 1) + lower_x;
	//							max_depth = std::max(max_depth, depth_buffers_[level - 1][lower_index]);
	//						}
	//					}
	//				}

	//				depth_buffers_[level][index] = std::max(depth_buffers_[level][index], max_depth);
	//			}
	//		}
	//	}
	//}

	void MultiLevelDepthBuffer::update_depth(uint32_t x, uint32_t y, float depth)
	{
		uint32_t index = y * width_ + x;
		depth_buffers_[0][index] = std::min(depth_buffers_[0][index], depth);

		for (uint32_t level = 1; level < levels_; ++level) {
			x >>= 1;
			y >>= 1;
			uint32_t level_width = width_ >> level;
			index = y * level_width + x;
			float& current_depth = depth_buffers_[level][index];

			if (current_depth <= depth) {
				break;
			}

			current_depth = depth;

			float min_depth = depth;
			for (int dy = 0; dy <= 1; ++dy) {
				for (int dx = 0; dx <= 1; ++dx) {
					uint32_t neighbour_x = (x << 1) + dx;
					uint32_t neighbour_y = (y << 1) + dy;
					if (neighbour_x < width_ && neighbour_y < height_) {
						uint32_t neighbour_index = neighbour_y * (level_width << 1) + neighbour_x;
						min_depth = std::min(min_depth, depth_buffers_[level - 1][neighbour_index]);
					}
				}
			}
			if (min_depth >= current_depth) {
				break;
			}
			depth = min_depth;
		}
	}

	bool MultiLevelDepthBuffer::is_occluded(uint32_t min_x, uint32_t max_x, uint32_t min_y, uint32_t max_y,
		float depth) const
	{
		uint32_t level = 0;
		while (level < levels_ - 1) {
			uint32_t level_width = width_ >> level;
			uint32_t level_height = height_ >> level;

			if ((max_x >> level) == (min_x >> level) && (max_y >> level) == (min_y >> level)) {
				break;
			}
			level++;
		}

		return depth_buffers_[level][get_index(min_x, min_y, level)] < depth;
	}

	void MultiLevelDepthBuffer::reset()
	{
		for (uint32_t i = 0; i < levels_; ++i)
		{
			std::fill(depth_buffers_[i].begin(), depth_buffers_[i].end(), std::numeric_limits<float>::infinity());
		}
	}

	float MultiLevelDepthBuffer::get_depth(uint32_t x, uint32_t y) const
	{
		return depth_buffers_[0][y * width_ + x];
	}

	void MultiLevelDepthBuffer::set_depth(uint32_t x, uint32_t y, float depth)
	{
		depth_buffers_[0][y * width_ + x] = depth;
	}

	uint32_t MultiLevelDepthBuffer::get_index(uint32_t x, uint32_t y, uint32_t level) const
	{
		uint32_t level_width = width_ >> level;
		uint32_t level_x = x >> level;
		uint32_t level_y = y >> level;
		return level_y * level_width + level_x;
	}
}
