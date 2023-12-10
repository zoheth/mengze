#include "hierarchical_zbuffer.h"

namespace
{
	float octree_transform_time;
}

namespace mengze
{
	HierarchicalZbufferRasterizer::HierarchicalZbufferRasterizer(Camera& camera, Geometry& geometry, bool use_octree): Rasterizer(camera, geometry), use_octree_(use_octree)
	{
		if (use_octree_)
		{
			timer_.reset();
			octree_ = std::make_unique<Octree>(geometry.get_vertices(), geometry.get_indices());
			octree_construct_time_ = timer_.elapsed();
		}
	}

	void HierarchicalZbufferRasterizer::resize_depth_buffer(uint32_t width, uint32_t height)
	{
		depth_mipmap_ = std::make_unique<DepthMipmap>(width, height);
	}

	void HierarchicalZbufferRasterizer::reset_depth_buffer()
	{
		if (depth_mipmap_)
			depth_mipmap_->reset();
	}

	void HierarchicalZbufferRasterizer::render_triangle()
	{
		if (octree_)
		{
			traverse_octree();
		}
		else
		{
			for (uint32_t i = 0; i < num_triangles_; ++i)
			{
				render_object(i);
			}
		}

	}

	ScreenBounds HierarchicalZbufferRasterizer::transform_to_screen_space(BoundingBox& bounds) const
	{
		auto min_screen = glm::vec3(std::numeric_limits<float>::max());
		auto max_screen = glm::vec3(std::numeric_limits<float>::min());
		float min_depth = std::numeric_limits<float>::max();

		int width = get_width();
		int height = get_height();

		for (int x = 0; x <= 1; x++) {
			for (int y = 0; y <= 1; y++) {
				for (int z = 0; z <= 1; z++) {
					glm::vec3 pt = glm::vec3(x ? bounds.max.x : bounds.min.x,
					                         y ? bounds.max.y : bounds.min.y,
					                         z ? bounds.max.z : bounds.min.z);

					pt = to_screen_space(pt);

					pt.x = glm::clamp(pt.x, 0.0f, static_cast<float>(width));
					pt.y = glm::clamp(pt.y, 0.0f, static_cast<float>(height));

					min_screen = glm::min(min_screen, pt);
					max_screen = glm::max(max_screen, pt);
					min_depth = std::min(min_depth, pt.z);
				}
			}
		}
		return { glm::vec2(min_screen),glm::vec2(max_screen),min_depth };
	}

	void HierarchicalZbufferRasterizer::traverse_octree()
	{
		OctreeNode* node = octree_->root_.get();
		octree_transform_time = 0.f;
		traverse_and_test(node);
		octree_to_screen_space_time_ = octree_transform_time;
	}

	void HierarchicalZbufferRasterizer::traverse_and_test(OctreeNode* node)
	{
		timer_.reset();
		auto screen_bounds = transform_to_screen_space(node->bounds);
		octree_transform_time += timer_.elapsed();

		if (!depth_mipmap_->is_occluded(screen_bounds.min.x, screen_bounds.max.x, screen_bounds.min.y, screen_bounds.max.y, screen_bounds.min_depth))
		{
			if (node->children[0] == nullptr)
			{
				for (auto index : node->triangle_indices)
				{
					render_object(index);
				}
			}
			else
			{
				for (auto& child : node->children)
				{
					if (child)
						traverse_and_test(child.get());
				}
			}
		}
	}

	void HierarchicalZbufferRasterizer::render_object(uint32_t index)
	{
		const auto& triangle = get_screen_triangle(index);
		if (!check_screen_triangle(triangle))
			return;
		const auto& world_triangle = get_world_triangle(index);

		rasterize_triangle(triangle, world_triangle);
	}

	void HierarchicalZbufferRasterizer::rasterize_triangle(const Triangle& triangle, const Triangle& world_triangle)
	{
		const glm::vec3& v0 = triangle.vertices[0];
		const glm::vec3& v1 = triangle.vertices[1];
		const glm::vec3& v2 = triangle.vertices[2];


		float min_x = std::min({ v0.x, v1.x, v2.x });
		float max_x = std::max({ v0.x, v1.x, v2.x });
		float min_y = std::min({ v0.y, v1.y, v2.y });
		float max_y = std::max({ v0.y, v1.y, v2.y });

		float min_depth = std::min({ v0.z,v1.z,v2.z });

		if (depth_mipmap_->is_occluded(min_x, max_x, min_y, max_y, min_depth))
			return;

		float area = Rasterizer::edge_func(v0, v1, v2);

		glm::vec3 normal = glm::normalize(glm::cross(world_triangle.vertices[1] - world_triangle.vertices[0], world_triangle.vertices[2] - world_triangle.vertices[0]));
		glm::vec3 color = simple_shading(normal);

		for (uint32_t y = min_y; y <= max_y; ++y) {
			for (uint32_t x = min_x; x <= max_x; ++x) {
				glm::vec3 p(x, y, 0);

				float w0 = edge_func(v1, v2, p);
				float w1 = edge_func(v2, v0, p);
				float w2 = edge_func(v0, v1, p);

				if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
					w0 /= area;
					w1 /= area;
					w2 /= area;

					float depth = w0 * v0.z + w1 * v1.z + w2 * v2.z;

					if (depth < depth_mipmap_->get_depth(x, y))
					{
						set_pixel(x, y, color);
						//depth_mipmap_->set_depth(x, y, depth);
						depth_mipmap_->update_depth(x, y, depth);
					}
				}
			}
		}
		// depth_mipmap_->update_higher_levels_depth(std::floor(min_x), std::ceil(max_x), std::floor(min_y), std::ceil(max_y));
	}
}
