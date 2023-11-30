#pragma once
#include "rendering/camera.h"
#include "rendering/renderer.h"

#include "edge_table.h"
#include "geometry.h"

namespace mengze
{
	float edge_func(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
		return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
	}


	class NaiveZbufferRenderer : public Renderer
	{
	public:
		explicit NaiveZbufferRenderer(Camera& camera, Geometry& geometry) : camera_(camera), geometry_(geometry)
		{
			screen_vertices_.resize(geometry_.get_vertices().size());
		}

		glm::vec3 transform_vertex(const glm::vec3& vertex) const
		{
			glm::vec4 v = camera_.get_view_matrix() * glm::vec4(vertex, 1.0f);
			v = camera_.get_projection_matrix() * v;
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
			clear(glm::vec3(0.1f));
			for (uint32_t i = 0; i < screen_vertices_.size(); ++i)
			{
				screen_vertices_[i] = transform_vertex(geometry_.get_vertices()[i]);
			}

			for (uint32_t i =0; i<geometry_.get_num_triangles(); ++i)
			{
				const auto& triangle = geometry_.get_triangle(i, &screen_vertices_);
				rasterize_triangle(triangle);
			}

		}
	private:
		void rasterize_triangle(const Triangle& triangle) {
			const glm::vec3& v0 = triangle.vertices[0];
			const glm::vec3& v1 = triangle.vertices[1];
			const glm::vec3& v2 = triangle.vertices[2];
			float min_x = std::min({ v0.x, v1.x, v2.x });
			min_x = std::max({ 0.f, min_x });
			float max_x = std::max({ v0.x, v1.x, v2.x });
			max_x = std::min({ static_cast<float>(get_width() - 1), max_x });
			float min_y = std::min({ v0.y, v1.y, v2.y });
			min_x = std::max({ 0.f, min_y });
			float max_y = std::max({ v0.y, v1.y, v2.y });
			max_y = std::min({ static_cast<float>(get_height() - 1), max_y });

			float area = edge_func(v0, v1, v2);

			for (uint32_t y = min_y; y <= max_y; ++y) {
				for (uint32_t x = min_x; x <= max_x; ++x) {
					glm::vec3 p(x, y, 0);

					float w0 = edge_func(v1, v2, p);
					float w1 = edge_func(v2, v0, p);
					float w2 = edge_func(v0, v1, p);

					if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
						w0 /= area;
						w1 /= area;
						w2 /= area;

						float depth = w0 * v0.z + w1 * v1.z + w2 * v2.z;

						set_pixel(x, y, glm::vec3(1));
					}
				}
			}
		}
	private:
		Camera& camera_;
		Geometry& geometry_;
		std::vector<glm::vec3> screen_vertices_;
		float* depth_buffer_{ nullptr };
	};
}