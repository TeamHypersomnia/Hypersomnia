#pragma once

template <class N, class R>
void setup_node_defaults(N& new_node, const R& resource) {
	static constexpr bool is_resource = std::is_same_v<remove_cref<decltype(resource.get_display_name())>, std::string>;
	static_assert(is_resource);

	if constexpr(std::is_same_v<R, editor_wandering_pixels_resource>) {
		static_cast<components::wandering_pixels&>(new_node) = resource.editable.node_defaults;
		new_node.size = resource.editable.default_size;
	}
	else if constexpr(std::is_same_v<R, editor_particles_resource>) {
		static_cast<particle_effect_modifier&>(new_node) = static_cast<const particle_effect_modifier&>(resource.editable);
	}
	else if constexpr(std::is_same_v<R, editor_sprite_resource>) {
		new_node.size.value = resource.editable.size;
		new_node.size.is_enabled = false;
	}
	else if constexpr(std::is_same_v<R, editor_prefab_resource>) {
		new_node = resource.editable.default_node_properties;
	}
}
