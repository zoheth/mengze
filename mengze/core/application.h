#pragma once

#include <string>
#include <memory>
#include <vector>

#include "layer.h"

struct GLFWwindow;

namespace mengze {
	struct ApplicationSpecification {
		std::string name = "App";
		uint32_t width = 1600;
		uint32_t height = 900;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& spec = ApplicationSpecification{});
		~Application();

		void init();
		void run();

	private:
		ApplicationSpecification spec_;
		GLFWwindow* window_ {nullptr};
		bool running_ {false};

		std::vector<std::unique_ptr<Layer>> layers_;
	};

	std::unique_ptr<Application> create_application(int argc, char** argv);
}