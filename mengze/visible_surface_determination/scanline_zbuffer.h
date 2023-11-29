#pragma once
#include "rendering/camera.h"
#include "rendering/renderer.h"

#include "edge_table.h"
#include "geometry.h"

namespace mengze
{
	class ZbufferRenderer : public Renderer
	{
	public:
		explicit ZbufferRenderer(Camera& camera, Geometry& geometry) : camera_(camera), geometry_(geometry)
		{
			screen_vertices_.resize(geometry_.get_vertices().size());
		}

		glm::vec3 transform_vertex(const glm::vec3& vertex) const
		{
			glm::vec4 v = camera_.get_projection_matrix() * camera_.get_view_matrix() * glm::vec4(vertex, 1.0f);
			v /= v.w;
			v.x = (v.x + 1.0f) * 0.5f * static_cast<float>(get_width());
			v.y = (1.0f - v.y) * 0.5f * static_cast<float>(get_height());
			return v;
		}

		void on_update(float ts) override
		{
			camera_.on_update(ts);
			if (depth_buffer_ && get_width() * get_height() > 0)
				std::fill_n(depth_buffer_, get_width() * get_height(), std::numeric_limits<float>::max());

			for(uint32_t i = 0;i<screen_vertices_.size();++i)
			{
				screen_vertices_[i] = transform_vertex(geometry_.get_vertices()[i]);
			}
		}

		void on_resize(uint32_t width, uint32_t height) override
		{
			camera_.on_resize(width, height);
			mengze::Renderer::on_resize(width, height);

			delete[] depth_buffer_;
			depth_buffer_ = new float[width * height];
			std::fill_n(depth_buffer_, width * height, std::numeric_limits<float>::max());
		}

		void render() override
		{
			EdgeTable edge_table(get_height());

			for (const auto& triangle : geometry_.get_triangles())
			{
				for (int i = 0; i < 3; ++i)
				{
					const glm::vec3& p1 = triangle.vertices[i];
					const glm::vec3& p2 = triangle.vertices[(i + 1) % 3];

					const float y_max = std::max(p1.y, p2.y);
					const float x_start = (p1.y < p2.y) ? p1.x : p2.x;
					constexpr float epsilon = 1e-6f;
					const float inverse_slope = std::abs(p2.y - p1.y) > epsilon ? (p2.x - p1.x) / (p2.y - p1.y) : 0;

					Edge edge(y_max, x_start, inverse_slope);
					edge_table.add_edge(edge);
				}
			}

			edge_table.sort();


			for (uint32_t y = 0; y < get_height(); ++y) {

				std::list<Edge> active_edge_table = edge_table[y];

				auto it = active_edge_table.begin();
				while (it != active_edge_table.end()) {
					const float x_start = it->x_current();
					++it;
					if (it == active_edge_table.end()) break;
					const float x_end = it->x_current();
					++it;

					for (auto x = static_cast<uint32_t>(x_start); x < static_cast<uint32_t>(x_end); ++x) {

						float pixel_depth = CalculatePixelDepth(x, y);

						if (pixel_depth < depth_buffer_[y * get_width() + x]) {
							depth_buffer_[y * get_width() + x] = pixel_depth;

							set_pixel(x, y, glm::vec3(1.0f - pixel_depth));
						}
					}
				}
				active_edge_table.remove_if([y](const Edge& edge) {
					return static_cast<uint32_t>(edge.y_max()) <= y;
					});

				for (auto& edge : active_edge_table) {
					edge.update_x_current();
				}

				edge_table.remove_edges_below(y + 1);
			}

		}

	private:
		Camera& camera_;
		Geometry& geometry_;
		std::vector<glm::vec3> screen_vertices_;
		float* depth_buffer_{ nullptr };
	};
}