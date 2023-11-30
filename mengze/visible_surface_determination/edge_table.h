#pragma once

#include <algorithm>
#include <vector>
#include <list>

#include "geometry.h"

namespace mengze
{
	struct EdgePair;

	struct Polygon
	{
		uint32_t id;
		float a, b, c, d;
		uint32_t num_y;
		std::array<glm::vec3, 3> vertices;
		EdgePair* p_edge_pair{nullptr};
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
		// std::vector<std::list<Edge>> edge_table;

		PolygonStorage() = delete;

		explicit PolygonStorage(uint32_t y)
		{
			polygon_table.resize(y);
			// edge_table.resize(y);
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
			polygon.vertices = vertices;

			polygon_table[y].push_back(polygon);

		}
	};

	struct EdgePair
	{
		Edge left;
		Edge right;
		float z_left;
		float delta_z_x;
		float delta_z_y;
		std::array<glm::vec3, 3> vertices;
		EdgePair(const Polygon& polygon)
		{
			vertices = polygon.vertices;

			std::sort(vertices.begin(), vertices.end(), [](const glm::vec3& a, const glm::vec3& b)
				{
					return a.y < b.y;
				});

			if(vertices[1].x < vertices[2].x)
			{
				left = Edge(vertices[0], vertices[1], polygon.id);
				right = Edge(vertices[0], vertices[2], polygon.id);
			}
			else
			{
				left = Edge(vertices[0], vertices[2], polygon.id);
				right = Edge(vertices[0], vertices[1], polygon.id);
			}

			z_left = -polygon.d - polygon.a * vertices[0].x - polygon.b * vertices[0].y;
			delta_z_x = -polygon.a / polygon.c;
			delta_z_y = -polygon.b / polygon.c;
		}
		// false to delete
		bool update()
		{
			left.num_y--;
			right.num_y--;

			if(left.num_y <= 0)
			{
				left=Edge(vertices[1], vertices[2], left.polygon_id);
			}
			else if(right.num_y <= 0)
			{
				right= Edge(vertices[1], vertices[2], left.polygon_id);
			}

		}
	};

	struct ActiveEdge
	{
		std::list<Polygon> polygons;
		std::list<EdgePair> edge_pairs;

		uint32_t cur_y = -1;

		void update(const PolygonStorage& polygon_storage)
		{
			cur_y++;
			for (auto& polygon : polygons)
			{
				polygon.num_y--;
				if (polygon.num_y <= 0)
				{
					polygons.remove(polygon);
				}
				else
				{
					polygon.p_edge_pair->update();
				}
				
			}

			for (auto polygen : polygon_storage.polygon_table[cur_y])
			{
				polygons.push_back(polygen);
				edge_pairs.emplace_back(polygen);
			}

		}
	};
}