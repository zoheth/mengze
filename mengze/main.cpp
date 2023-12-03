#include "imgui.h"
#include "core/application.h"
#include "rendering/renderer.h"
#include "rendering/render_layer.h"

#include "visible_surface_determination/scanline_zbuffer.h"
#include "visible_surface_determination/zbuffer.h"
#include "visible_surface_determination/ui_layer.h"

class SimpleRenderer : public mengze::Renderer
{
public:
	void render() override
	{
		for (uint32_t y = 0; y < get_height(); ++y)
		{
			for (uint32_t x = 0; x < get_width(); ++x)
			{
				set_pixel(x, y,
					{
						static_cast<float>(x) / static_cast<float>(get_width()),
						static_cast<float>(y) / static_cast<float>(get_height()),
						0.5f
					});
			}
		}
	}
};

class SettingsLayer :public mengze::Layer
{
public:
	SettingsLayer(mengze::RenderLayer* render_layer) : Layer("Settings"), render_layer_(render_layer)
	{

	}

	void push_rasterizer(const std::string& name, mengze::Rasterizer* rasterizer)
	{
		rasterizer_names.push_back(name);
		rasterizer_ptrs.push_back(rasterizer);
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

			if (ImGui::Combo("", &rasterizer_index_, items.c_str())) {
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
		if (const auto scanline_rasterizer = dynamic_cast<mengze::ScanlineZbufferRasterizer*>(rasterizer))
		{
			ImGui::Text("Find intersections time: %.3f ms", scanline_rasterizer->get_find_intersections_time());
			ImGui::Text("Construct time: %.3f ms", scanline_rasterizer->get_construct_time());
		}
		ImGui::End();
	}

private:
	mengze::RenderLayer* render_layer_{ nullptr };
	int rasterizer_index_{ 0 };
	std::vector<std::string> rasterizer_names;
	std::vector<mengze::Rasterizer*> rasterizer_ptrs;

};

std::unique_ptr<mengze::Application> mengze::create_application(int argc, char** argv)
{
	return std::make_unique<mengze::Application>();
}

int main(int argc, char** argv)
{
	auto app = mengze::create_application(argc, argv);

	//app->push_layer<mengze::RenderLayer>(std::make_unique<SimpleRenderer>());
	auto camera = mengze::Camera(45.0f, 0.1f, 100.0f);
	auto geometry = mengze::Geometry("scenes\\bunny.obj");
	//auto geometry = mengze::Geometry("scenes\\bathroom\\bathroom.obj");

	const auto zbuffer_rasterizer = std::make_unique<mengze::ZbufferRasterizer>(camera, geometry);
	const auto scanline_zbuffer_rasterizer = std::make_unique<mengze::ScanlineZbufferRasterizer>(camera, geometry);


	auto* render_layer = dynamic_cast<mengze::RenderLayer*>(app->push_layer<mengze::RenderLayer>(zbuffer_rasterizer.get()));

	auto* settings_layer = dynamic_cast<SettingsLayer*>(app->push_layer<SettingsLayer>(render_layer));

	settings_layer->push_rasterizer("Z buffer", zbuffer_rasterizer.get());
	settings_layer->push_rasterizer("Scanline z buffer", scanline_zbuffer_rasterizer.get());

	app->run();
}