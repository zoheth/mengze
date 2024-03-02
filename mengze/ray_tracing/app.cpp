#include "app.h"

Renderer::Renderer() :
    mengze::Renderer(),
    camera_({13.f, 2.f, 3.f}, {-13.f, -2.f, -3.f}, 20.0f)
{
}

void Renderer::set_scene(mengze::Scene *scene)
{
	scene_ = scene;
}

void Renderer::on_resize(uint32_t width, uint32_t height)
{
	camera_.on_resize(width, height);
	mengze::Renderer::on_resize(width, height);
}

void Renderer::render()
{
	camera_.initialize();
	for (uint32_t y = cur_y_; y < get_height(); ++y)
	{
		for (uint32_t x = 0; x < get_width(); ++x)
		{
			glm::vec3 color{0.0f, 0.0f, 0.0f};
			for (uint32_t sample = 0; sample < 10; ++sample)
			{
				mengze::Ray ray = camera_.get_ray(x, y);
				color += ray_color(ray, 10);
			}
			color /= 10.0f;

			set_pixel(x, y, color);
		}
		if (y - cur_y_ > 20)
		{
			cur_y_ = y;
			break;
		}
	}
}

glm::vec3 Renderer::ray_color(const mengze::Ray &r, int depth) const
{
	if (depth <= 0)
		return glm::vec3{0, 0, 0};

	mengze::HitRecord rec;

	if (scene_->hit(r, mengze::Interval(0.001), rec))
	{
	    mengze::Ray scattered;
	    glm::vec3   attenuation;

	    if (rec.material->scatter(r, rec, attenuation, scattered))
	        return attenuation * ray_color(scattered, depth - 1);

	    return {0.f, 0.f, 0.f};
	}

	glm::vec3 unit_direction = glm::normalize(r.direction());
	float     a              = 0.5f * (unit_direction.y + 1.0f);
	return (1.0f - a) * glm::vec3(1.0, 1.0, 1.0) + a * glm::vec3(0.5, 0.7, 1.0);
}
