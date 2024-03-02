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
	auto          ground_material = std::make_shared<mengze::Lambertian>(glm::vec3(0.5f, 0.5f, 0.5f));

	scene.add(std::make_shared<mengze::Sphere>(glm::vec3(0.0f, -1000.0f, 0.0f), 1000.0f, ground_material));

	for (int a = -11; a < 11; ++a)
	{
		for (int b = -11; b < 11; ++b)
		{
			float     choose_mat = mengze::random_float();
			glm::vec3 center(a + 0.9f * mengze::random_float(), 0.2f, b + 0.9f * mengze::random_float());

			if (glm::length(center - glm::vec3(4.0f, 0.2f, 0.0f)) > 0.9f)
			{
				std::shared_ptr<mengze::Material> sphere_material;

				if (choose_mat < 0.8f)
				{
					// diffuse
					glm::vec3 albedo = mengze::random_vec3() * mengze::random_vec3();
					sphere_material  = std::make_shared<mengze::Lambertian>(albedo);
					scene.add(std::make_shared<mengze::Sphere>(center, 0.2f, sphere_material));
				}
				else if (choose_mat < 0.95f)
				{
					// metal
					glm::vec3 albedo = mengze::random_vec3(0.5f, 1.0f);
					float     fuzz   = mengze::random_float(0.0f, 0.5f);
					sphere_material  = std::make_shared<mengze::Metal>(albedo, fuzz);
					scene.add(std::make_shared<mengze::Sphere>(center, 0.2f, sphere_material));
				}
				else
				{
					// glass
					sphere_material = std::make_shared<mengze::Dielectric>(1.5f);
					scene.add(std::make_shared<mengze::Sphere>(center, 0.2f, sphere_material));
				}
			}
		}
	}

	auto material1 = std::make_shared<mengze::Dielectric>(1.5f);
	scene.add(std::make_shared<mengze::Sphere>(glm::vec3(0.0f, 1.0f, 0.0f), 1.0f, material1));

	auto material2 = std::make_shared<mengze::Lambertian>(glm::vec3(0.4f, 0.2f, 0.1f));
	scene.add(std::make_shared<mengze::Sphere>(glm::vec3(-4.0f, 1.0f, 0.0f), 1.0f, material2));

	auto material3 = std::make_shared<mengze::Metal>(glm::vec3(0.7f, 0.6f, 0.5f), 0.0f);
	scene.add(std::make_shared<mengze::Sphere>(glm::vec3(4.0f, 1.0f, 0.0f), 1.0f, material3));

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