#include "ray_tracing/renderer.h"

#include "core/logging.h"

namespace mengze::rt
{
Renderer::Renderer(const std::shared_ptr<mengze::rt::Camera> &camera) :
    mengze::Renderer(),
    camera_(camera)
{
}

Renderer::Renderer(const std::shared_ptr<mengze::rt::Camera> &camera, uint32_t sample_per_pixel, int max_depth) :
    mengze::Renderer(),
    camera_(camera),
    sample_per_pixel_(sample_per_pixel),
    max_depth_(max_depth)
{
}

void Renderer::set_scene(const std::shared_ptr<mengze::rt::Scene> &scene)
{
	scene_ = scene;
}

void Renderer::on_resize(uint32_t width, uint32_t height)
{
	camera_->on_resize(width, height);
	mengze::Renderer::on_resize(width, height);
}

void Renderer::render()
{
	camera_->initialize();
	if (cur_y_ >= get_height())
	{
		cur_y_ = 0;
		LOGW("Reset cur_y_ to 0")
	}
	uint32_t y = cur_y_;
	for (; y < get_height(); ++y)
	{
		for (uint32_t x = 0; x < get_width(); ++x)
		{
			glm::vec3 color{0.0f, 0.0f, 0.0f};
			for (uint32_t sample = 0; sample < sample_per_pixel_; ++sample)
			{
				Ray ray = camera_->get_ray(x, y);
				color += ray_color(ray, max_depth_);
			}
			color /= sample_per_pixel_;

			set_pixel(x, y, color);
		}
		if (y - cur_y_ > 20)
		{
			break;
		}
	}
	cur_y_ = y;
}

glm::vec3 Renderer::ray_color(const Ray &r, int depth) const
{
	if (depth <= 0)
		return glm::vec3{0, 0, 0};

	HitRecord rec;

	if (!scene_->world().hit(r, Interval(0.001f), rec))
	{
		return glm::vec3{0, 0, 0};
	}

	ScatterRecord scatter_record;
	glm::vec3     color_from_emission = rec.material->emitted(rec.u, rec.v, rec.position);

	if (!rec.material->scatter(r, rec, scatter_record))
		return color_from_emission;

	if (scatter_record.skip_pdf)
	{
		return scatter_record.attenuation * ray_color(scatter_record.skip_pdf_ray, depth - 1);
	}

	auto light = std::make_shared<HittablePdf>(scene_->lights(), rec.position);
	MixturePdf p(light, scatter_record.pdf);

	Ray scattered = Ray(rec.position, p.generate());
	auto pdf_val  = p.value(scattered.direction());

	float scattering_pdf = rec.material->scattering_pdf(r, rec, scattered);

	glm::vec3 sample_color = ray_color(scattered, depth - 1);
	glm::vec3 color_from_scatter = scatter_record.attenuation * scattering_pdf * sample_color / pdf_val;

	return color_from_emission + color_from_scatter;
}
}        // namespace mengze::rt
