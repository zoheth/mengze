#pragma once

#include <vector>
#include <list>

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

	class Edge
	{
	public:
		Edge(float y_max, float x_start, float inverse_slope)
			: y_max_(y_max), x_current_(x_start), inverse_slope_(inverse_slope) {}

		float y_max() const { return y_max_; }
		float x_current() const { return x_current_; }
		float inverse_slope() const { return inverse_slope_; }

		void update_x_current() { x_current_ += inverse_slope_; }

	private:
		float y_max_;
		float x_current_;
		float inverse_slope_;
	};

	class EdgeTable
	{
	public:
		explicit EdgeTable(uint32_t height) : table_(height) {}

		void add_edge(const Edge& edge)
		{
			uint32_t y = static_cast<uint32_t>(edge.y_max());
			if (y < static_cast<uint32_t>(table_.size()))
			{
				table_[y].push_back(edge);
			}
		}

		const std::list<Edge>& operator[](uint32_t y) const { return table_[y]; }

		void sort()
		{
			for (auto& row : table_)
			{
				row.sort([](const Edge& lhs, const Edge& rhs) {
					return lhs.x_current() < rhs.x_current();
					});
			}
		}

		void update_x_current()
		{
			for (auto& row : table_)
			{
				for (auto& edge : row)
				{
					edge.update_x_current();
				}
			}
		}

		void remove_edges_below(uint32_t y)
		{
			table_.erase(table_.begin(), table_.begin() + y);
		}


	private:
		std::vector<std::list<Edge>> table_;
	};
}