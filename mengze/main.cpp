#include "imgui.h"
#include "core/application.h"
#include "rendering/renderer.h"
#include "rendering/render_layer.h"

#include "visible_surface_determination/scanline_zbuffer.h"
#include "visible_surface_determination/naive_zbuffer.h"

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
	//app->push_layer<mengze::RenderLayer>(std::make_unique<mengze::ScanlineZbufferRenderer>(camera, geometry));
	app->push_layer<mengze::RenderLayer>(std::make_unique<mengze::NaiveZbufferRasterizer>(camera, geometry));
	app->run();
}