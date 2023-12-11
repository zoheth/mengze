#include "imgui.h"

#include <glm/gtc/matrix_transform.hpp>

#include "core/application.h"
#include "rendering/renderer.h"
#include "rendering/render_layer.h"

#include "visible_surface_determination/scanline_zbuffer.h"
#include "visible_surface_determination/zbuffer.h"
#include "visible_surface_determination/hierarchical_zbuffer.h"
#include "visible_surface_determination/gui.h"

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
#ifdef NDEBUG
	auto camera = mengze::Camera(45.0f, 0.1f, 5000.0f, 1000.0f);
	glm::mat4 rotation_matrix = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	rotation_matrix = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * rotation_matrix;
	auto geometry = mengze::Geometry("scenes\\luyu.obj", &rotation_matrix);
#else
	auto camera = mengze::Camera(45.0f, 0.1f, 100.0f);
	auto geometry = mengze::Geometry("scenes\\bunny.obj");
#endif

	const auto zbuffer_rasterizer = std::make_unique<mengze::ZbufferRasterizer>(camera, geometry);
	const auto scanline_zbuffer_rasterizer = std::make_unique<mengze::ScanlineZbufferRasterizer>(camera, geometry);
	const auto hierarchical_zbuffer_rasterizer = std::make_unique<mengze::HierarchicalZbufferRasterizer>(camera, geometry);
	const auto hierarchical_zbuffer_octree_rasterizer = std::make_unique<mengze::HierarchicalZbufferRasterizer>(camera, geometry, true);


	auto* render_layer = dynamic_cast<mengze::RenderLayer*>(app->push_layer<mengze::RenderLayer>(zbuffer_rasterizer.get()));

	auto* settings_layer = dynamic_cast<mengze::SettingsLayer*>(app->push_layer<mengze::SettingsLayer>(render_layer));

	settings_layer->push_rasterizer("Z buffer", zbuffer_rasterizer.get());
	settings_layer->push_rasterizer("Scanline z buffer", scanline_zbuffer_rasterizer.get());
	settings_layer->push_rasterizer("Hierarchical z buffer", hierarchical_zbuffer_rasterizer.get());
	settings_layer->push_rasterizer("Hierarchical z buffer with octree", hierarchical_zbuffer_octree_rasterizer.get());

	app->run();
}