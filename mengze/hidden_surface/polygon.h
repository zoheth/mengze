#pragma once

#include <algorithm>
#include <vector>
#include <list>

#include "geometry.h"

namespace mengze
{
	struct IntersectionResult {
		float left_x;
		float right_x;
		double left_z;
		double z_increment;
	};


	struct Polygon {

		Polygon(const Triangle& triangle);

		[[nodiscard]] bool find_intersections(double y, IntersectionResult& result) const;

		std::array<glm::vec3, 3> vertices;
		int lifetime;
		std::array<double, 3> slopes;
		uint32_t id;

	private:
		inline void calculate_slopes();

		inline static double calculate_slope(const glm::vec3& p1, const glm::vec3& p2);

		inline static void check_and_add_intersection(const glm::vec3& p1, const glm::vec3& p2, double y, double slope,
		                                       std::vector<glm::vec3>& intersections);
	};

	struct PolygonStorage
	{
		std::vector<std::list<Polygon>> polygon_table;
		// std::vector<std::list<Edge>> edge_table;

		PolygonStorage() = delete;

		explicit PolygonStorage(uint32_t y)
		{
			polygon_table.resize(y);
			// edge_table.resize(y);
		}

		void add_triangle(const Triangle& triangle, uint32_t index)
		{
			float y_max = std::max({ triangle.vertices[0].y, triangle.vertices[1].y, triangle.vertices[2].y });
			float y_min = std::min({ triangle.vertices[0].y, triangle.vertices[1].y, triangle.vertices[2].y });
			Polygon polygon(triangle);
			polygon.lifetime = std::floor(y_max) - std::ceil(y_min) + 1;
			polygon.id = index;
			if(polygon.lifetime>0)
				polygon_table[std::ceil(y_min)].push_back(polygon);

		}
	};

	struct ActivePolygon
	{
		std::list<Polygon> polygons;

		void update(const PolygonStorage& polygon_storage, uint32_t cur_y)
		{
			for (auto it = polygons.begin(); it != polygons.end(); /* no increment here */)
			{
				it->lifetime--;
				if (it->lifetime <= 0)
				{
					it = polygons.erase(it);
				}
				else
				{
					++it;
				}
			}

			for (auto polygon : polygon_storage.polygon_table[cur_y])
			{
				polygons.push_back(polygon);
			}

		}
	};
}