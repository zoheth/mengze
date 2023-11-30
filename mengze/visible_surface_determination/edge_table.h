#pragma once

#include <algorithm>
#include <vector>
#include <list>

#include "geometry.h"

namespace mengze
{
	struct Polygon
	{
		uint32_t id;
		float a, b, c, d;
		uint32_t num_y;
	};

	struct Edge
	{
		uint32_t polygon_id;
		uint32_t x_start;
		float delta_x;
		uint32_t num_y;

		Edge(glm::vec3 v0, glm::vec3 v1, uint32_t id)
			// need v0.y < v1.y
		{
			x_start = v0.x;
			float epsilon = 0.0001f;
			delta_x = (v1.x - v0.x) / ((v1.y - v0.y) + epsilon);
			num_y = v1.y - v0.y;
			polygon_id = id;
		}
	};

	struct PolygonStorage
	{
		std::vector<std::list<Polygon>> polygon_table;
		std::vector<std::list<Edge>> edge_table;

		PolygonStorage() = delete;

		explicit PolygonStorage(uint32_t y)
		{
			polygon_table.resize(y);
			edge_table.resize(y);
		}

		void add_triangle(const Triangle& triangle, uint32_t index)
		{
			auto vertices = triangle.vertices;

			std::sort(vertices.begin(), vertices.end(), [](const glm::vec3& a, const glm::vec3& b) {
				return a.y < b.y;
				});

			glm::vec3 edge1 = vertices[1] - vertices[0];
			glm::vec3 edge2 = vertices[2] - vertices[0];

			glm::vec3 normal = glm::cross(edge1, edge2);
			normal = glm::normalize(normal);
			Polygon polygon;
			polygon.id = index;
			polygon.a = normal.x;
			polygon.b = normal.y;
			polygon.c = normal.z;
			polygon.d = -glm::dot(normal, vertices[0]);

			auto y = static_cast<uint32_t>(vertices[0].y);
			auto max_y = static_cast<uint32_t>(vertices[2].y);
			polygon.num_y = max_y - y + 1;

			polygon_table[y].push_back(polygon);


			edge_table[y].emplace_back(vertices[0], vertices[1], index);
			edge_table[y].emplace_back(vertices[0], vertices[2], index);
			edge_table[static_cast<uint32_t>(vertices[1].y)].emplace_back(vertices[1], vertices[2], index);

		}
	};



	struct ActiveEdge
	{
		std::list<Polygon> polygons;
		std::list<Edge> edges;

		uint32_t cur_y=-1;

		void update(const PolygonStorage& polygon_storage)
		{
			cur_y++;
			for (auto& polygon : polygons)
			{
				polygon.num_y--;
				if(polygon.num_y<=0)
				{
					polygons.remove(polygon);
				}
			}

			for (auto polygen : polygon_storage.polygon_table[cur_y])
			{
				polygons.push_back(polygen);
			}

		}
	};
}