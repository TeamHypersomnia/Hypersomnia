#pragma once

template <class N, class R>
void setup_node_defaults(N& new_node, const R& resource) {
	if constexpr(std::is_same_v<R, editor_wandering_pixels_resource>) {
		static_cast<components::wandering_pixels&>(new_node.editable) = resource.editable.node_defaults;
	}
}
