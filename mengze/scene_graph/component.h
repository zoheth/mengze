#pragma once
#include <string>
#include <typeindex>

namespace mengze
{
	class Component
	{
	public:
		Component() = default;

		explicit Component(std::string name) : name_(std::move(name)) {}

		virtual ~Component() = default;

		Component(const Component&) = delete;
		Component& operator=(const Component&) = delete;
		// Move constructor don't need to be deleted
		Component(Component&&) = default;
		Component& operator=(Component&&) = delete;

		const std::string& get_name() const { return name_; }

		virtual std::type_index get_type() const = 0;

	private:
		std::string name_;
	};
}
