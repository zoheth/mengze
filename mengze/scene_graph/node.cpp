#include "node.h"

#include "components/transform.h"

namespace mengze
{
	Node::Node(const size_t id, const std::string& name)
		: id_{ id }, name_{ name }, transform_{ *this }
	{
		set_component(transform_);
	}

	void Node::set_parent(Node& parent)
	{
		parent_ = &parent;

		transform_.invalidate_world_matrix();
	}

	void Node::add_child(Node& child)
	{
		children_.push_back(&child);
	}

	void Node::set_component(Component& component)
	{
		if (const auto it = components_.find(component.get_type()); it != components_.end())
		{
			it->second = &component;
		}
		else
		{
			components_.insert({ component.get_type(), &component });
		}
	}

	Component& Node::get_component(const std::type_index& type) const
	{
		return *components_.at(type);
	}

	bool Node::has_component(const std::type_index& type) const
	{
		return components_.count(type) > 0;
	}
}
