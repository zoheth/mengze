#pragma once

#include <random>
#include <iostream>

#include <spdlog/fmt/fmt.h>

#include "ray.h"

#include "ray_tracing/hittable.h"
#include "rendering/camera.h"
#include "util/math.h"

namespace mengze::rt
{
class Camera : public mengze::Camera
{
  public:
	Camera(glm::vec3 position, glm::vec3 forward, float fov);

	void render(const Hittable &scene)
	{
		for (uint32_t i = 0; i < viewport_width_; ++i)
		{
			for (uint32_t j = 0; j < viewport_height_; ++j)
			{
				glm::vec3 color{0.0f, 0.0f, 0.0f};
				for (uint32_t sample = 0; sample < samples_per_pixel_; ++sample)
				{
					Ray ray = get_ray(i, j);
					color += ray_color(ray, max_depth_, scene);
				}
				std::cout<< fmt::format("{},{},{}\n", color.r, color.g, color.b);
			}
		}
	}

	Ray get_ray(float i, float j) const
	{
		auto pixel_center = pixel00_loc_ + (i * pixel_delta_u_) + (j * pixel_delta_v_);
		auto pixel_sample = pixel_center + pixel_sample_square();

		auto ray_origin    = position_;
		auto ray_direction = pixel_sample - ray_origin;

		return Ray(ray_origin, ray_direction);
	}

  private:
	void initialize()
	{
		auto theta = glm::radians(fov_);
		auto h     = glm::tan(theta / 2);

		auto projection_plane_height = 2 * h * focus_distance_;
		auto projection_plane_width  = projection_plane_height * (static_cast<float>(viewport_width_) / viewport_height_);

		w_ = glm::normalize(forward_direction_);
		v_ = glm::normalize(up_direction_);
		u_ = glm::normalize(glm::cross(v_, w_));

		glm::vec3 camera_u = projection_plane_width * u_;
		glm::vec3 camera_v = projection_plane_height * -v_;

		pixel_delta_u_ = camera_u / static_cast<float>(viewport_width_);
		pixel_delta_v_ = camera_v / static_cast<float>(viewport_height_);

		pixel00_loc_ = position_ - (focus_distance_ * w_) - camera_u / 2.f - camera_v / 2.f;
		pixel00_loc_ += 0.5f * (pixel_delta_u_ + pixel_delta_v_);

		auto defocus_redius = focus_distance_ * glm::tan(glm::radians(defocus_angle_ / 2.0f));
		defocus_disk_u_     = u_ * defocus_redius;
		defocus_disk_v_     = v_ * defocus_redius;
	}

	glm::vec3 pixel_sample_square() const
	{
		float px = -0.5f + random_float(0.0f, 1.0f);
		float py = -0.5f + random_float(0.0f, 1.0f);

		return (px * pixel_delta_u_) + (py * pixel_delta_v_);
	}

	glm::vec3 ray_color(const Ray &r, int depth, const Hittable &scene) const
	{
		if (depth <= 0)
			return glm::vec3{0, 0, 0};

		HitRecord rec;

		if (scene.hit(r, Interval(0.001), rec))
		{
			Ray       scattered;
			glm::vec3 attenuation;

			if (rec.material->scatter(r, rec, attenuation, scattered))
				return attenuation * ray_color(scattered, depth - 1, scene);

			return {0.f, 0.f, 0.f};
		}
	}

  private:
	uint32_t samples_per_pixel_{10};
	uint32_t max_depth_{10};

	float focus_distance_{10.0f};
	float defocus_angle_{0.6f};

	glm::vec3 pixel00_loc_;

	glm::vec3 pixel_delta_u_;
	glm::vec3 pixel_delta_v_;

	glm::vec3 u_, v_, w_;

	glm::vec3 defocus_disk_u_;
	glm::vec3 defocus_disk_v_;
};
}        // namespace mengze::rt
