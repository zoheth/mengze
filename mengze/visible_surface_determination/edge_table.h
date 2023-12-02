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

		Polygon(const Triangle& triangle)
			: vertices{ triangle.vertices } {
			calculate_slopes();
		}

		[[nodiscard]] bool find_intersections(double y, IntersectionResult& result) const {
			std::vector<glm::vec3> intersections;
			check_and_add_intersection(vertices[0], vertices[1], y, slopes[0], intersections);
			check_and_add_intersection(vertices[1], vertices[2], y, slopes[1], intersections);
			check_and_add_intersection(vertices[2], vertices[0], y, slopes[2], intersections);

			// Ensure exactly two points, duplicating if necessary.
			if (intersections.size() == 1) {
				intersections.push_back(intersections.front());
			}
			if(intersections.size() != 2)
			{
				return false;
			}

			std::sort(intersections.begin(), intersections.end(),
				[](const glm::vec3& a, const glm::vec3& b) { return a.x < b.x; });

			float left = std::round(intersections[0].x);
			float right = std::round(intersections[1].x);

			// Calculate z value and z increment
			double left_z = intersections[0].z;
			double z_increment = (intersections[1].z - intersections[0].z) / (right - left);

			result.left_x = left;
			result.right_x = right;
			result.left_z = left_z;
			result.z_increment = z_increment;
			return true;
		}

		std::array<glm::vec3, 3> vertices;
		int lifetime;
		std::array<double, 3> slopes;
		uint32_t id;

	private:
		void calculate_slopes() {
			slopes[0] = calculate_slope(vertices[0], vertices[1]);
			slopes[1] = calculate_slope(vertices[1], vertices[2]);
			slopes[2] = calculate_slope(vertices[2], vertices[0]);
		}

		static double calculate_slope(const glm::vec3& p1, const glm::vec3& p2) {
			if (p2.y == p1.y) return std::numeric_limits<double>::infinity();
			return (p2.x - p1.x) / (p2.y - p1.y);
		}

		static void check_and_add_intersection(const glm::vec3& p1, const glm::vec3& p2, double y, double slope,
			std::vector<glm::vec3>& intersections) {

			if ((p1.y - y) * (p2.y - y) <= 0) { // Check if y is between p1.y and p2.y

				double x = p1.x + (y - p1.y) * slope;
				double z = p1.z + (x - p1.x) * (p2.z - p1.z) / (p2.x - p1.x);  // Linear interpolation for z
				intersections.emplace_back(x, y, z);
			}
		}
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
			polygon.lifetime = std::floor(y_max) - std::ceil(y_min)+1;
			polygon.id = index;
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