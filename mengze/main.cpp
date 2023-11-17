#include "imgui.h"
#include "core/application.h"
#include "rendering/renderer.h"
#include "rendering/render_layer.h"

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

int main(int argc, char** argv)
{
	auto app = mengze::create_application(argc, argv);

	app->push_layer<mengze::RenderLayer>(std::make_unique<SimpleRenderer>());
	app->run();
}