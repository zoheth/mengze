#include <thread>
#include <memory>

#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>

#include "core/application.h"
#include "rendering/render_layer.h"
#include "rendering/renderer.h"
#include "ray_tracing/app.h"


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
				          {static_cast<float>(x) / static_cast<float>(get_width()),
				           static_cast<float>(y) / static_cast<float>(get_height()),
				           0.5f});
			}
		}
	}
};

std::unique_ptr<mengze::Application> mengze::create_application(int argc, char **argv)
{
	return std::make_unique<mengze::Application>();
}

int main(int argc, char **argv)
{
	auto app = mengze::create_application(argc, argv);

	mengze::rt::ray_tracing_app_setup(*app);

	app->run();
}