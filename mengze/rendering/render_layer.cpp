#include "render_layer.h"

#include "imgui.h"

namespace mengze
{
	void RenderLayer::on_ui_render()
	{
		ImGui::Begin("Viewport");
		viewport_width_ = static_cast<uint32_t>(ImGui::GetContentRegionAvail().x);
		viewport_height_ = static_cast<uint32_t>(ImGui::GetContentRegionAvail().y);

		renderer_->on_resize(viewport_width_, viewport_height_);
		auto image = renderer_->render();

		if (image)
		{
			ImGui::Image(image->get_descriptor_set(), ImVec2(viewport_width_, viewport_height_), ImVec2(0, 1), ImVec2(1, 0));
		}
		ImGui::End();
	}
}
