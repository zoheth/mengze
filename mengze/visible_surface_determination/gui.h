#pragma once

#include "core/layer.h"
#include "rendering/render_layer.h"

#include "rasterizer.h"
#include "scanline_zbuffer.h"

namespace mengze
{
	class SettingsLayer :public Layer
	{
	public:
		SettingsLayer(RenderLayer* render_layer) : Layer("Settings"), render_layer_(render_layer)
		{

		}

		void push_rasterizer(const std::string& name, Rasterizer* rasterizer)
		{
			rasterizer_names.push_back(name);
			rasterizer_ptrs.push_back(rasterizer);
		}

		void on_attach() override
		{
			ImGuiIO& io = ImGui::GetIO();
			io.FontGlobalScale = 1.4f;
		}

		void on_ui_render() override
		{
			auto* rasterizer = rasterizer_ptrs[rasterizer_index_];

			ImGui::Begin("Settings");

			if (ImGui::CollapsingHeader("Rasterizers")) {

				std::string items;
				for (const auto& name : rasterizer_names) {
					items += name + '\0';
				}

				if (ImGui::Combo("Choose Rasterizer", &rasterizer_index_, items.c_str())) {
					render_layer_->set_renderer(rasterizer_ptrs[rasterizer_index_]);
				}
			}
			ImGui::End();

			ImGui::Begin("Statistics");
			if (rasterizer_ptrs[rasterizer_index_])
			{
				ImGui::Text("Vertex transform time: %.3f ms", rasterizer->get_vertex_transform_time());
				ImGui::Text("Rasterization time: %.3f ms", rasterizer->get_rasterization_time());
				ImGui::Text("Triangle count: %d", rasterizer->get_triangle_count());
				ImGui::Text("Pixel count: %d x %d", rasterizer->get_height(), rasterizer->get_width());

			}
			if (const auto scanline_rasterizer = dynamic_cast<ScanlineZbufferRasterizer*>(rasterizer))
			{
				ImGui::Text("Find intersections time: %.3f ms", scanline_rasterizer->get_find_intersections_time());
				ImGui::Text("Construct time: %.3f ms", scanline_rasterizer->get_construct_time());
			}
			ImGui::End();
		}

	private:
		RenderLayer* render_layer_{ nullptr };
		int rasterizer_index_{ 0 };
		std::vector<std::string> rasterizer_names;
		std::vector<Rasterizer*> rasterizer_ptrs;

	};
}
