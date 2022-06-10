#include "application/setups/editor/project/editor_project.h"

std::unordered_map<std::string, editor_layer*> editor_project::make_name_to_layer_map() {
	std::unordered_map<std::string, editor_layer*> out;

	for (auto& layer : layers) {
		out[layer.name] = std::addressof(layer);
	}

	return out;
}
