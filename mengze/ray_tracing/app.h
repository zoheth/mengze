#pragma once

#include "rendering/renderer.h"
#include "rendering/render_layer.h"
#include "camera.h"
#include "sphere.h"
#include "scene.h"


class Renderer : public mengze::Renderer
{
public:
	Renderer();

	void set_scene(mengze::Scene* scene);

	void on_resize(uint32_t width, uint32_t height) override;
	void render() override;

	glm::vec3 ray_color(const mengze::Ray &r, int depth) const;

private:
	mengze::Scene* scene_ {nullptr};
	mengze::rt::Camera camera_;

	uint32_t cur_y_ = 0;
};