#include <thread>

#include "imgui.h"
#include <glm/gtc/matrix_transform.hpp>

#include "core/application.h"
#include "ray_tracing/app.h"
#include "ray_tracing/camera.h"
#include "ray_tracing/material.h"
#include "ray_tracing/scene.h"
#include "ray_tracing/sphere.h"
#include "rendering/render_layer.h"
#include "rendering/renderer.h"

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
	mengze::Scene scene;

	auto red = std::make_shared<mengze::Lambertian>(glm::vec3{.65, .05, .05});
	auto white = std::make_shared<mengze::Lambertian>(glm::vec3{.73, .73, .73});
	auto green = std::make_shared<mengze::Lambertian>(glm::vec3{.12, .45, .15});
	auto light = std::make_shared<mengze::DiffuseLight>(glm::vec3{15, 15, 15});



	//auto camera = mengze::rt::Camera({13.f, 2.f, 3.f}, {-13.f, -2.f, -3.f}, 20.0f);

	/*std::thread render_thread([scene, &camera]() {
		camera.render(scene);
	});*/

	//camera.render(scene);

	auto app = mengze::create_application(argc, argv);

	auto renderer = std::make_shared<Renderer>();
	renderer->set_scene(&scene);

	auto *render_layer = dynamic_cast<mengze::RenderLayer *>(
	    app->push_layer<mengze::RenderLayer>(renderer.get()));

	app->run();
}