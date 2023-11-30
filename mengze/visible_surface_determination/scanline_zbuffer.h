#pragma once
#include "rendering/camera.h"
#include "rendering/renderer.h"

#include "edge_table.h"
#include "geometry.h"

namespace mengze
{
	class ScanlineZbufferRenderer : public Renderer
	{
	public:
		explicit ScanlineZbufferRenderer(Camera& camera, Geometry& geometry) : camera_(camera), geometry_(geometry)
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


		}

	private:
		Camera& camera_;
		Geometry& geometry_;
		std::vector<glm::vec3> screen_vertices_;
		float* depth_buffer_{ nullptr };
	};
}