#include "render_layer.h"

#include "imgui.h"

namespace mengze
{
	void RenderLayer::on_ui_render()
	{
		if (!renderer_)
			return;
		ImGui::Begin("Viewport");
		viewport_width_ = static_cast<uint32_t>(ImGui::GetContentRegionAvail().x);
		viewport_height_ = static_cast<uint32_t>(ImGui::GetContentRegionAvail().y);
		if (viewport_height_ == 0)
			viewport_height_ = 16;

		renderer_->on_resize(viewport_width_, viewport_height_);
		renderer_->render();
		renderer_->present();

		auto image = renderer_->get_film();

		if (image)
		{
			ImGui::Image(image->get_descriptor_set(), ImVec2(viewport_width_, viewport_height_), ImVec2(0, 1), ImVec2(1, 0));
		}
		ImGui::End();
	}
}
