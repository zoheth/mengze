#pragma once

#include <algorithm>
#include <vector>
#include <list>
#include <cmath>

#include "geometry.h"

namespace mengze
{
	class TriangleTable {
	public:
		explicit TriangleTable(uint32_t height)
			: table_(height){}

		void AddTriangle(const Triangle& triangle) {
			float min_y = std::min({ triangle.vertices[0].y, triangle.vertices[1].y, triangle.vertices[2].y });
			float max_y = std::max({ triangle.vertices[0].y, triangle.vertices[1].y, triangle.vertices[2].y });

			min_y = std::max(min_y, 0.0f);
			max_y = std::min(max_y, static_cast<float>(table_.size()) - 1);

			for (auto y = static_cast<uint32_t>(min_y); y <= static_cast<uint32_t>(max_y); ++y) {
				table_[y].push_back(triangle);
			}
		}

		void clear(uint32_t y) {
			table_[y].clear();
		}

		const std::list<Triangle>& operator[](uint32_t y) const {
			return table_[y];
		}

	private:
		std::vector<std::list<Triangle>> table_;
	};

	struct Edge
	{
		float x_start;
		float delta_x;
		float remain_height;

		float z_start;
		float delta_z_x;
		float delta_z_y;
	};

	class ActiveEdgeTable {
	public:
		void add_edge_pair(const Triangle& triangle)
		{
			auto vertices = triangle.vertices;
			std::sort(vertices.begin(), vertices.end(), [](const glm::vec3& v1, const glm::vec3& v2) {
				return v1.y < v2.y;
			});

			for (uint32_t i =0; i<3;++i)
			{
				const uint32_t next = (i + 1) % 3;

				if (vertices[i].y == vertices[next].y)
				{
					continue;
				}
				if(scanline_y_ >= std::min(vertices[i].y, vertices[next].y)
					&& scanline_y_ <= std::max(vertices[i].y, vertices[next].y))
				{
					Edge edge;
					edge.remain_height = std::abs(vertices[i].y - vertices[next].y);
					edge.x_start = vertices[i].x;
					edge.delta_x = (vertices[next].x - vertices[i].x) / edge.remain_height;
					edge.z_start = vertices[i].z;
					//edge.delta_z_x = (vertices[next].z - vertices[i].z) / std::abs();
					edge.delta_z_y = (vertices[next].z - vertices[i].z) / edge.remain_height;
					edges_.push_back(edge);
				}
			}
			assert(edges_.size() % 2 == 0);
		}

	private:
		std::list<Edge> edges_;
		uint32_t scanline_y_;
	};
}