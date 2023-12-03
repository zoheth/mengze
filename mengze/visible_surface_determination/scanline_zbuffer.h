#pragma once
#include "rendering/camera.h"
#include "rasterizer.h"

#include "core/timer.h"
#include "polygon.h"
#include "geometry.h"

namespace mengze
{
	inline Timer timer;
	class ScanlineZbufferRasterizer : public Rasterizer
	{
	public:
		ScanlineZbufferRasterizer(Camera& camera, Geometry& geometry) : Rasterizer(camera, geometry)
		{
		}

		float get_find_intersections_time() const { return find_intersections_time_; }
		float get_construct_time() const { return construct_time_; }

		void render_triangle() override
		{
			timer.reset();
			PolygonStorage polygon_storage(get_height());

			for (uint32_t i = 0; i < num_triangles_; ++i)
			{
				const auto& triangle = get_screen_triangle(i);
				if (!check_screen_triangle(triangle))
					continue;
				polygon_storage.add_triangle(triangle, i);
			}
			construct_time_ = timer.elapsed();


			ActivePolygon active_polygon;

			float  find_intersections_time = 0.0f;

			for (uint32_t y = 0; y < get_height(); ++y)
			{
				active_polygon.update(polygon_storage, y);
				for (auto& polygon : active_polygon.polygons)
				{
					IntersectionResult info{};

					timer.reset();
					bool result = polygon.find_intersections(y, info);
					find_intersections_time += timer.elapsed();
					if(!result)
						continue;
					

					float depth = info.left_z;
					for (uint32_t x = info.left_x; x < info.right_x; ++x)
					{
						if (depth < depth_buffer_[y * get_width() + x])
						{
							depth_buffer_[y * get_width() + x] = depth;
							Triangle world_triangle = get_world_triangle(polygon.id);
							glm::vec3 normal = glm::normalize(glm::cross(world_triangle.vertices[1] - world_triangle.vertices[0], world_triangle.vertices[2] - world_triangle.vertices[0]));
							set_pixel(x, y, simple_shading(normal));
						}
						depth += info.z_increment;
					}
				}
			}
			find_intersections_time_ = find_intersections_time;

		}
	private:
		float find_intersections_time_;
		float construct_time_;

	};
}