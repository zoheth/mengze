#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "scene_graph/components/transform.h"

namespace mengze
{
	class Node
	{
	public:
		Node(const size_t id, std::string name);

		virtual ~Node() = default;

size_t get_id() const { return id_; }

		const std::string& get_name() const { return name_; }

		Transform& get_transform() { return transform_; }

		void set_parent(Node& parent);

		Node* get_parent() const { return parent_; }

		void add_child(Node& child);

		const std::vector<Node*>& get_children() const { return children_; }

		void set_component(Component& component);

		template<class T>
		T& get_component()
		{
			return dynamic_cast<T&>(get_component(typeid(T)));
		}

		Component& get_component(const std::type_index& type) const;

		template<class T>
		bool has_component() const
		{
			return has_component(typeid(T));
		}

		bool has_component(const std::type_index& type) const;


	private:
		size_t id_;
		std::string name_;

		Transform transform_;

		Node* parent_{ nullptr };

		std::vector<Node*> children_;

		std::unordered_map<std::type_index, Component*> components_;
	};
}
