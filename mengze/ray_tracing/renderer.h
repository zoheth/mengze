#pragma once

#include "core/timer.h"
#include "rendering/renderer.h"
#include "ray_tracing/camera.h"
#include "ray_tracing/scene.h"

namespace mengze::rt
{
class Renderer : public mengze::Renderer
{
  public:
	explicit Renderer(const std::shared_ptr<mengze::rt::Camera> &camera);

	Renderer(const std::shared_ptr<mengze::rt::Camera> &camera, uint32_t sample_per_pixel, int max_depth);

	void set_scene(const std::shared_ptr<mengze::rt::Scene> &scene);

	void on_resize(uint32_t width, uint32_t height) override;
	void render() override;

	glm::vec3 ray_color(const Ray &r, int depth) const;

  private:
	Timer timer_;
	std::shared_ptr<mengze::rt::Scene> scene_{nullptr};
	std::shared_ptr<mengze::rt::Camera> camera_{nullptr};

	uint32_t sample_per_pixel_ = 10;
	int max_depth_ = 10;

	uint32_t cur_y_ = 0;
};
}
