#pragma once
#include <glm/glm.hpp>

#include "core/timer.h"
#include "rendering/camera.h"
#include "rasterizer.h"
#include "geometry.h"
#include "depth_mipmap.h"
#include "octree.h"


namespace mengze
{
	struct ScreenBounds {
		glm::vec2 min;
		glm::vec2 max;
		float min_depth;
	};

	class HierarchicalZbufferRasterizer : public Rasterizer
	{
	public:
		HierarchicalZbufferRasterizer(Camera& camera, Geometry& geometry, bool use_octree = false);

		void resize_depth_buffer(uint32_t width, uint32_t height) override;

		void reset_depth_buffer() override;

		void render_triangle() override;

		bool use_octree() const { return use_octree_; }

		float get_octree_construct_time() const { return octree_construct_time_; }
		float get_octree_to_screen_space_time() const { return octree_to_screen_space_time_; }

	private:
		ScreenBounds transform_to_screen_space(BoundingBox& bounds) const;

		void traverse_octree();

		void traverse_and_test(OctreeNode* node);

		void render_object(uint32_t index);

		void rasterize_triangle(const Triangle& triangle, const Triangle& world_triangle);

	private:
		Timer timer_;

		std::unique_ptr<DepthMipmap> depth_mipmap_{ nullptr };

		bool use_octree_{ false };

		std::unique_ptr<Octree> octree_{ nullptr };

		float octree_construct_time_;
		float octree_to_screen_space_time_;

	};
}
