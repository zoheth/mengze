#pragma once

#include "core/layer.h"
#include "renderer.h"

namespace mengze
{
	class RenderLayer : public Layer
	{
	public:
		RenderLayer() = delete;
		explicit RenderLayer(std::unique_ptr<Renderer> renderer) : Layer("Render"), renderer_(std::move(renderer)) {}

		void on_update(float ts) override
		{
			renderer_->on_update(ts);
		}

		void on_ui_render() override;

	private:
		std::unique_ptr<Renderer> renderer_;
		uint32_t viewport_width_{ 0 };
		uint32_t viewport_height_{ 0 };
	};
}