#include "ray_tracing/renderer.h"

#include <execution>

#include "core/logging.h"
#include "core/timer.h"
#include "ray_tracing/pdf.h"

 namespace
{
 std::atomic<int> pixels_rendered{0};
 int              total_pixels;
 void print_progress()
{
	while (pixels_rendered < total_pixels)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		int progress = 100.0 * pixels_rendered / total_pixels;
		std::cout << "\rProgress: " << progress << "%" << std::flush;
	}
	std::cout << "\r" << std::flush;
 }

 }

namespace mengze::rt
{
Renderer::Renderer(const std::shared_ptr<mengze::rt::Camera> &camera) :
    mengze::Renderer(),
    camera_(camera)
{
	is_accumulation_ = true;
}

Renderer::Renderer(const std::shared_ptr<mengze::rt::Camera> &camera, uint32_t sample_per_pixel, int max_depth) :
    mengze::Renderer(),
    camera_(camera),
    sample_per_pixel_(sample_per_pixel),
    max_depth_(max_depth)
{
	is_accumulation_ = true;
}

void Renderer::set_scene(const std::shared_ptr<mengze::rt::Scene> &scene)
{
	scene_ = scene;
}

void Renderer::on_resize(uint32_t width, uint32_t height)
{
	camera_->on_resize(width, height);
	camera_->initialize();
	mengze::Renderer::on_resize(width, height);
}

void Renderer::on_update(float ts)
{
	camera_->on_update(ts);

	if (camera_->is_dirty())
	{
		frame_index_ = 1;
		camera_->set_dirty(false);
	}
}

void Renderer::render()
{
	if (frame_index_ == 1)
	{
		reset_accumulation();
	}
	if (frame_index_ > sample_per_pixel_)
	{
		return;
	}

	LOGI("Rendering frame: {}", frame_index_)
#define MULTITHREAD_RENDER 1

#if MULTITHREAD_RENDER
	std::thread progress_thread(print_progress);

	total_pixels = get_width() * get_height();

	std::for_each(std::execution::par, image_vertical_iter_.begin(), image_vertical_iter_.end(), [this](uint32_t y) {
		std::for_each(std::execution::par, image_horizontal_iter_.begin(), image_horizontal_iter_.end(), [this, y](uint32_t x) {
			Ray       ray   = camera_->get_ray(x, y);
			glm::vec3 color = ray_color(ray, max_depth_);
			get_pixel_accumulation(x, y) += color;
			glm::vec3 accumulated_color = get_pixel_accumulation(x, y);
			accumulated_color /= static_cast<float>(frame_index_);

			set_pixel(x, y, accumulated_color);
			++pixels_rendered;
		});
	});

	progress_thread.join();
	pixels_rendered = 0;
#else

	for (uint32_t y = 0; y < get_height(); ++y)
	{
		for (uint32_t x = 0; x < get_width(); ++x)
		{
			Ray       ray   = camera_->get_ray(x, y);
			glm::vec3 color = ray_color(ray, max_depth_);
			get_pixel_accumulation(x, y) += color;
			glm::vec3 accumulated_color = get_pixel_accumulation(x, y);
			accumulated_color /= static_cast<float>(frame_index_);

			set_pixel(x, y, accumulated_color);
		}
	}
#endif
	if (is_accumulation_)
	{
		frame_index_++;
	}
}

glm::vec3 Renderer::ray_color(const Ray &r, int depth) const
{
	if (scene_->lights().empty())
	{
		LOGE("No light in the scene");
		return glm::vec3{0, 0, 0};
	}

	if (depth <= 0)
		return glm::vec3{0, 0, 0};

	HitRecord rec;

	if (!scene_->world().hit(r, Interval(0.001f), rec))
	{
		return glm::vec3{0, 0, 0};
	}

	// return glm::vec3{1, 0, 0};
	ScatterRecord scatter_record;
	glm::vec3     color_from_emission = rec.material->emitted(rec.u, rec.v, rec.position);

	if (!rec.material->scatter(r, rec, scatter_record))
		return color_from_emission;

	if (scatter_record.skip_pdf)
	{
		return scatter_record.attenuation * ray_color(scatter_record.skip_pdf_ray, depth - 1);
	}

	auto       light = std::make_shared<HittablePdf>(scene_->lights(), rec.position);
	//MixturePdf p(light, scatter_record.pdf);

	Ray  scattered = Ray(rec.position, scatter_record.pdf->generate());
	auto pdf_val   = scatter_record.pdf->value(scattered.direction());

	float scattering_pdf = rec.material->scattering_pdf(r, rec, scattered);

	glm::vec3 sample_color       = ray_color(scattered, depth - 1);
	glm::vec3 color_from_scatter = scatter_record.attenuation * scattering_pdf * sample_color / pdf_val;

	return color_from_emission + color_from_scatter;
}
}        // namespace mengze::rt
