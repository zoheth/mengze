#pragma once

#include "core/layer.h"
#include "rendering/render_layer.h"

#include "rasterizer.h"
#include "scanline_zbuffer.h"
#include "hierarchical_zbuffer.h"

namespace mengze
{
	class SettingsLayer :public Layer
	{
	public:
		SettingsLayer(RenderLayer* render_layer) : Layer("Settings"), render_layer_(render_layer)
		{

		}

		void push_rasterizer(const std::string& name, std::unique_ptr<Rasterizer> rasterizer)
		{
			rasterizer_names.push_back(name);
			rasterizers.push_back(std::move(rasterizer));
		}

		void on_attach() override
		{
			ImGuiIO& io = ImGui::GetIO();
			io.FontGlobalScale = 1.4f;
		}

		void on_ui_render() override
		{
		    auto *rasterizer = rasterizers[rasterizer_index_].get();

			ImGui::Begin("Settings");

			if (ImGui::CollapsingHeader("Rasterizers")) {

				std::string items;
				for (const auto& name : rasterizer_names) {
					items += name + '\0';
				}

				if (ImGui::Combo("Choose Rasterizer", &rasterizer_index_, items.c_str())) {
				    render_layer_->set_renderer(rasterizers[rasterizer_index_].get());
				}
			}
			ImGui::End();

			ImGui::Begin("Statistics");
		    if (rasterizers[rasterizer_index_])
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

			if(const auto hierarchical_rasterizer = dynamic_cast<HierarchicalZbufferRasterizer*>(rasterizer))
			{
				if(hierarchical_rasterizer->use_octree())
				{
					ImGui::Text("Octree construct time: %.3f ms", hierarchical_rasterizer->get_octree_construct_time());
					ImGui::Text("Octree to screen space time: %.3f ms", hierarchical_rasterizer->get_octree_to_screen_space_time());
				}
			}

			ImGui::End();
		}

	private:
		RenderLayer* render_layer_{ nullptr };
		int rasterizer_index_{ 0 };
		std::vector<std::string> rasterizer_names;
		std::vector<std::unique_ptr<Rasterizer>> rasterizers;

	};
}
