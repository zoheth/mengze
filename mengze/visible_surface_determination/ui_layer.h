#pragma once

#include "core/layer.h"

#include "rasterizer.h"
#include "scanline_zbuffer.h"

namespace mengze
{
	class UiLayer : public Layer
	{
	public:
		UiLayer(Rasterizer* rasterizer)
		{
			rasterizer_ = rasterizer;
		}
		void on_ui_render() override
		{
			ImGui::Begin("Statistics");
			if (rasterizer_)
			{

				ImGui::Text("Vertex transform time: %.3f ms", rasterizer_->get_vertex_transform_time());
				ImGui::Text("Rasterization time: %.3f ms", rasterizer_->get_rasterization_time());
				ImGui::Text("Triangle count: %d", rasterizer_->get_triangle_count());
				ImGui::Text("Pixel count: %d x %d", rasterizer_->get_height(), rasterizer_->get_width());

			}
			if(const auto scanline_rasterizer = dynamic_cast<ScanlineZbufferRasterizer*>(rasterizer_))
			{
				ImGui::Text("Find intersections time: %.3f ms", scanline_rasterizer->get_find_intersections_time());
				ImGui::Text("Construct time: %.3f ms", scanline_rasterizer->get_construct_time());
			}
			ImGui::End();
		}

	private:
		Rasterizer* rasterizer_{ nullptr };
	};
}
