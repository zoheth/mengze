#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "component.h"
#include "node.h"

namespace mengze
{

	/*class Scene
	{
	public:
		Scene() = default;
		~Scene() = default;

		 Scene(const Scene&) = delete;
		 Scene& operator=(const Scene&) = delete;
		 Scene(Scene&&) = delete;
		 Scene& operator=(Scene&&) = delete;

	private:
		std::string name_;

		std::vector<std::unique_ptr<Node>> nodes_;

		Node* root_{ nullptr };

		std::unordered_map<std::type_index, std::vector<std::unique_ptr<Component>>> components_;
	};*/
}
