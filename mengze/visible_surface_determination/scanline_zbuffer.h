#pragma once
#include "rendering/camera.h"
#include "rasterizer.h"

#include "edge_table.h"
#include "geometry.h"

namespace mengze
{
	class ScanlineZbufferRasterizer : public Rasterizer
	{
	public:
		ScanlineZbufferRasterizer(Camera& camera, Geometry& geometry) : Rasterizer(camera, geometry)
		{
		}

		void render_triangle() override
		{

			PolygonStorage polygon_storage(get_height());

			for (uint32_t i = 0; i < num_triangles_; ++i)
			{
				const auto& triangle = get_screen_triangle(i);
				if (!check_screen_triangle(triangle))
					continue;
				polygon_storage.add_triangle(triangle, i);
			}

			ActivePolygon active_polygon;

			for (uint32_t y = 0; y < get_height(); ++y)
			{
				active_polygon.update(polygon_storage, y);
				for (auto& polygon : active_polygon.polygons)
				{
					IntersectionResult info{};
					if(!polygon.find_intersections(y,info))
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

		}

	};
}