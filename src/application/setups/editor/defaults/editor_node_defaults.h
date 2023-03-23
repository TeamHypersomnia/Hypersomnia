#pragma once

template <class N, class R>
void setup_node_defaults(N& new_node, const R& resource) {
	if constexpr(std::is_same_v<R, editor_wandering_pixels_resource>) {
		static_cast<components::wandering_pixels&>(new_node.editable) = resource.editable.node_defaults;
		new_node.editable.size = resource.editable.default_size;
	}
	else if constexpr(std::is_same_v<R, editor_sprite_resource>) {
		new_node.editable.size.value = resource.editable.size;
		new_node.editable.size.is_enabled = false;
	}
	else if constexpr(std::is_same_v<R, editor_prefab_resource>) {
		new_node.editable = resource.editable.default_node_properties;
	}
}
