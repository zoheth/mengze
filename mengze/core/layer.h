#pragma once

namespace mengze {
	class Layer
	{
	public:
		explicit Layer(std::string name = "Layer") : name_(std::move(name)) {}
		virtual ~Layer() = default;

		virtual void on_attach() {}
		virtual void on_detach() {}
		virtual void on_update() {}
		virtual void on_ui_render() {}
		virtual void on_event() {}

		const std::string& name() const { return name_; }

	protected:
		std::string name_;
	};
}