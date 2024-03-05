#include "app.h"

void hidden_surface_app_setup(mengze::Application &app)
{

	// app->push_layer<mengze::RenderLayer>(std::make_unique<SimpleRenderer>());
#ifdef NDEBUG
	auto camera   = mengze::Camera(45.0f, 0.1f, 5000.0f, 100.0f);
	auto geometry = mengze::Geometry("scenes\\sponza.obj");
#else
	auto camera   = mengze::Camera(45.0f, 0.1f, 100.0f);
	auto geometry = mengze::Geometry("scenes\\bunny.obj");
#endif

	auto zbuffer_rasterizer =
		std::make_shared<mengze::ZbufferRasterizer>(camera, geometry);
	auto scanline_zbuffer_rasterizer =
	    std::make_shared<mengze::ScanlineZbufferRasterizer>(camera, geometry);
	auto hierarchical_zbuffer_rasterizer =
	    std::make_shared<mengze::HierarchicalZbufferRasterizer>(camera, geometry);
	auto hierarchical_zbuffer_octree_rasterizer =
	    std::make_shared<mengze::HierarchicalZbufferRasterizer>(camera, geometry,
		                                                        true);

	auto *render_layer = dynamic_cast<mengze::RenderLayer *>(
		app.push_layer<mengze::RenderLayer>(zbuffer_rasterizer));

	auto *settings_layer = dynamic_cast<mengze::SettingsLayer *>(
		app.push_layer<mengze::SettingsLayer>(render_layer));

	settings_layer->push_rasterizer("Z buffer", std::shared_ptr<mengze::Rasterizer>(zbuffer_rasterizer));
	settings_layer->push_rasterizer("Scanline z buffer",
	                                std::shared_ptr<mengze::Rasterizer>(scanline_zbuffer_rasterizer));
	settings_layer->push_rasterizer("Hierarchical z buffer",
	                                std::shared_ptr<mengze::Rasterizer>(hierarchical_zbuffer_rasterizer));
	settings_layer->push_rasterizer("Hierarchical z buffer with octree",
	                                std::shared_ptr<mengze::Rasterizer>(hierarchical_zbuffer_octree_rasterizer));

	app.run();
}