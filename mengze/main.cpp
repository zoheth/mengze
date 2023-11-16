#include "imgui.h"
#include "core/application.h"
#include "rendering/renderer.h"

class SimpleRenderer : public mengze::Renderer
{
public:
	glm::vec4  calculate_color(uint32_t x, uint32_t y) override
	{
		return { static_cast<float>(x) / static_cast<float>(get_width()), static_cast<float>(y) / static_cast<float>(get_height()), 0.5f, 1.0f };
	}
};

std::unique_ptr<mengze::Application> mengze::create_application(int argc, char** argv)
{
	return std::make_unique<mengze::Application>();
}

class SandboxLayer : public mengze::Layer
{
	public:
	SandboxLayer() : Layer("Sandbox") {}
	void on_update() override
	{
	}

	void on_ui_render() override
	{
		ImGui::Begin("Viewport");
		viewport_width_ = static_cast<uint32_t>(ImGui::GetContentRegionAvail().x);
		viewport_height_ = static_cast<uint32_t>(ImGui::GetContentRegionAvail().y);

		renderer_.on_resize(viewport_width_, viewport_height_);
		auto image = renderer_.render();

		if(image)
		{
			ImGui::Image((void*)image->get_descriptor_set(), ImVec2(viewport_width_, viewport_height_), ImVec2(0, 1), ImVec2(1, 0));
		}
		ImGui::End();
	}

private:
	uint32_t viewport_width_ {0};
	uint32_t viewport_height_ {0};

	SimpleRenderer renderer_;
};

int main(int argc, char** argv)
{
	auto app = mengze::create_application(argc, argv);
	app->push_layer<SandboxLayer>();
	app->run();
}