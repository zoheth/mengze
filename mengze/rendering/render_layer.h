#pragma once

#include "core/layer.h"
#include "renderer.h"

namespace mengze
{
	class RenderLayer : public Layer
	{
	public:
		RenderLayer() = delete;
	  explicit RenderLayer(const std::shared_ptr<Renderer> &renderer) :
	      Layer("Render"), renderer_(renderer)
	  {}

		void set_renderer(const std::shared_ptr<Renderer> &renderer)
	  {
		  renderer_ = renderer;
	  }

		void on_update(float ts) override
		{
			if (renderer_)
				renderer_->on_update(ts);
		}

		void on_ui_render() override;

	private:
		std::shared_ptr<Renderer> renderer_{nullptr};
		uint32_t viewport_width_{ 0 };
		uint32_t viewport_height_{ 0 };
	};
}