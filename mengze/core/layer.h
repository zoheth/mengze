#pragma once

namespace mengze {
	class Layer
	{
	public:
		Layer(const std::string& name = "Layer") : name_(name) {}
		virtual ~Layer();

		virtual void on_attach() {}
		virtual void on_detach() {}
		virtual void on_update() {}
		virtual void on_ui_render() {}
		virtual void on_event() {}

		inline const std::string& name() const { return name_; }

	protected:
		std::string name_;
	};
}