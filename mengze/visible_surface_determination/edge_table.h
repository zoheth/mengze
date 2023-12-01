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
		float x_start;
		float delta_x;
		uint32_t num_y;

		Edge() = default;

		Edge(glm::vec3 v0, glm::vec3 v1)
			// need v0.y < v1.y
		{
			x_start = v0.x;
			float epsilon = 0.0001f;
			delta_x = (v1.x - v0.x) / ((v1.y - v0.y) + epsilon);
			num_y = static_cast<uint32_t>(v1.y - v0.y);
		}
	};

	struct EdgePair
	{
		Edge left;
		Edge right;
		Edge candidate;
		float x_left;
		float x_right;
		float z_left;
		float delta_z_x;
		float delta_z_y;
		glm::vec3 normal;
		EdgePair(const Polygon& polygon)
		{
			auto vertices = polygon.vertices;
			std::sort(vertices.begin(), vertices.end(), [](const glm::vec3& a, const glm::vec3& b)
				{
					return a.y < b.y;
				});

			if (vertices[1].x < vertices[2].x)
			{
				left = Edge(vertices[0], vertices[1]);
				right = Edge(vertices[0], vertices[2]);
			}
			else
			{
				left = Edge(vertices[0], vertices[2]);
				right = Edge(vertices[0], vertices[1]);
			}
			candidate = Edge(vertices[1], vertices[2]);

			normal = glm::cross(vertices[1] - vertices[0], vertices[2] - vertices[0]);

			x_left = left.x_start;
			x_right = right.x_start;


			z_left = -polygon.d - polygon.a * vertices[0].x - polygon.b * vertices[0].y;
			delta_z_x = -polygon.a / polygon.c;
			delta_z_y = -polygon.b / polygon.c;
		}
		// false to delete
		void update()
		{
			left.num_y--;
			right.num_y--;

			if (left.num_y <= 0)
			{
				left = candidate;
			}
			else if (right.num_y <= 0)
			{
				right = candidate;
			}
			x_left += left.delta_x;
			x_right += right.delta_x;
			z_left += delta_z_x * left.delta_x + delta_z_y;
			

		}
	};

	struct PolygonStorage
	{
		std::vector<std::list<Polygon>> polygon_table;
		// std::vector<std::list<Edge>> edge_table;
		uint32_t max_y;

		PolygonStorage() = delete;

		explicit PolygonStorage(uint32_t y)
		{
			max_y = y-1;
			polygon_table.resize(y);
			// edge_table.resize(y);
		}

		void add_triangle(const Triangle& triangle, uint32_t index)
		{
			auto vertices = triangle.vertices;

			std::sort(vertices.begin(), vertices.end(), [](const glm::vec3& a, const glm::vec3& b) {
				return a.y < b.y;
				});
			if(vertices[0].y<0 || vertices[2].y>max_y)
			{
				return;
			}

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

	struct ActivePolygon
	{
		std::list<Polygon> polygons;

		uint32_t cur_y = -1;

		void update(const PolygonStorage& polygon_storage)
		{
			cur_y++;
			for (auto it = polygons.begin(); it != polygons.end(); /* no increment here */)
			{
				it->num_y--;
				if (it->num_y <= 0)
				{
					delete it->p_edge_pair;
					it = polygons.erase(it);
				}
				else
				{
					it->p_edge_pair->update();
					++it;
				}
			}

			for (auto polygon : polygon_storage.polygon_table[cur_y])
			{
				polygon.p_edge_pair = new EdgePair(polygon);
				polygons.push_back(polygon);
			}

		}
	};
}