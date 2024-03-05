#include "ray_tracing/app.h"

#include "rendering/render_layer.h"
#include "ray_tracing/bvh.h"
#include "ray_tracing/triangle.h"
#include "ray_tracing/sphere.h"
#include "ray_tracing/scene.h"
#include "ray_tracing/camera.h"
#include "ray_tracing/renderer.h"

namespace mengze::rt
{

void ray_tracing_app_setup(mengze::Application &app)
{
	auto scene = std::make_shared<Scene>();

	auto red   = std::make_shared<Lambertian>(glm::vec3{.65, .05, .05});
	auto white = std::make_shared<Lambertian>(glm::vec3{.73, .73, .73});
	auto green = std::make_shared<Lambertian>(glm::vec3{.12, .45, .15});
	auto light = std::make_shared<DiffuseLight>(glm::vec3{15, 15, 15});

	scene->add(std::make_shared<Triangle>(glm::vec3(555, 0, 0), glm::vec3(555, 555, 0), glm::vec3(555, 0, 555), green));
	scene->add(std::make_shared<Triangle>(glm::vec3(555, 555, 0), glm::vec3(555, 555, 555), glm::vec3(555, 0, 555), green));

	/*scene->add(std::make_shared<Quad>(glm::vec3(555, 0, 0), glm::vec3(0, 555, 0), glm::vec3(0, 0, 555), green));*/

	scene->add(std::make_shared<Triangle>(glm::vec3(0, 0, 0), glm::vec3(0, 555, 0), glm::vec3(0, 0, 555), red));
	scene->add(std::make_shared<Triangle>(glm::vec3(0, 555, 0), glm::vec3(0, 555, 555), glm::vec3(0, 0, 555), red));

	scene->add(std::make_shared<Triangle>(glm::vec3(0, 0, 0), glm::vec3(555, 0, 0), glm::vec3(0, 0, 555), white));
	scene->add(std::make_shared<Triangle>(glm::vec3(555, 0, 0), glm::vec3(555, 0, 555), glm::vec3(0, 0, 555), white));

	scene->add(std::make_shared<Triangle>(glm::vec3(0, 0, 555), glm::vec3(555, 0, 555), glm::vec3(0, 555, 555), white));
	scene->add(std::make_shared<Triangle>(glm::vec3(555, 0, 555), glm::vec3(555, 555, 555), glm::vec3(0, 555, 555), white));

	scene->add(std::make_shared<Triangle>(glm::vec3(0, 555, 0), glm::vec3(555, 555, 0), glm::vec3(0, 555, 555), white));
	scene->add(std::make_shared<Triangle>(glm::vec3(555, 555, 0), glm::vec3(555, 555, 555), glm::vec3(0, 555, 555), white));

	scene->add(std::make_shared<Triangle>(glm::vec3(213, 554, 227), glm::vec3(343, 554, 227), glm::vec3(343, 554, 332), light));
	scene->add(std::make_shared<Triangle>(glm::vec3(213, 554, 227), glm::vec3(343, 554, 332), glm::vec3(213, 554, 332), light));

	/*scene->add(std::make_shared<Quad>(glm::vec3(343, 554, 332), glm::vec3(-130, 0, 0), glm::vec3(0, 0, -105), light));*/

	auto camera = std::make_shared<Camera>(glm::vec3{278.f, 278.f, -800.f}, glm::vec3{0.f, 0.f, 800.f}, 40.0f);

	auto renderer = std::make_shared<Renderer>(camera, 100, 10);
	renderer->set_scene(scene);

	auto *render_layer = dynamic_cast<mengze::RenderLayer *>(
	    app.push_layer<mengze::RenderLayer>(renderer));

}


void random_spheres()
{
	Scene scene;
	auto              ground_material = std::make_shared<Lambertian>(glm::vec3(0.5f, 0.5f, 0.5f));

	scene.add(std::make_shared<Sphere>(glm::vec3(0.0f, -1000.0f, 0.0f), 1000.0f, ground_material));

	for (int a = -11; a < 11; ++a)
	{
		for (int b = -11; b < 11; ++b)
		{
			float     choose_mat = mengze::random_float();
			glm::vec3 center(a + 0.9f * mengze::random_float(), 0.2f, b + 0.9f * mengze::random_float());

			if (glm::length(center - glm::vec3(4.0f, 0.2f, 0.0f)) > 0.9f)
			{
				std::shared_ptr<Material> sphere_material;

				if (choose_mat < 0.8f)
				{
					// diffuse
					glm::vec3 albedo = mengze::random_vec3() * mengze::random_vec3();
					sphere_material  = std::make_shared<Lambertian>(albedo);
					scene.add(std::make_shared<Sphere>(center, 0.2f, sphere_material));
				}
				else if (choose_mat < 0.95f)
				{
					// metal
					glm::vec3 albedo = mengze::random_vec3(0.5f, 1.0f);
					float     fuzz   = mengze::random_float(0.0f, 0.5f);
					sphere_material  = std::make_shared<Metal>(albedo, fuzz);
					scene.add(std::make_shared<Sphere>(center, 0.2f, sphere_material));
				}
				else
				{
					// glass
					sphere_material = std::make_shared<Dielectric>(1.5f);
					scene.add(std::make_shared<Sphere>(center, 0.2f, sphere_material));
				}
			}
		}
	}

	auto material1 = std::make_shared<Dielectric>(1.5f);
	scene.add(std::make_shared<Sphere>(glm::vec3(0.0f, 1.0f, 0.0f), 1.0f, material1));

	auto material2 = std::make_shared<Lambertian>(glm::vec3(0.4f, 0.2f, 0.1f));
	scene.add(std::make_shared<Sphere>(glm::vec3(-4.0f, 1.0f, 0.0f), 1.0f, material2));

	auto material3 = std::make_shared<Metal>(glm::vec3(0.7f, 0.6f, 0.5f), 0.0f);
	scene.add(std::make_shared<Sphere>(glm::vec3(4.0f, 1.0f, 0.0f), 1.0f, material3));

	// auto camera = Camera({13.f, 2.f, 3.f}, {-13.f, -2.f, -3.f}, 20.0f);
}
}
