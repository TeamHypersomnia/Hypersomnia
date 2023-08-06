#pragma once
#include "application/setups/editor/editor_official_resource_map.h"
#include "application/setups/editor/resources/editor_resource_id.h"

template <class S, class T>
auto& editor_official_resource_map::get_container(S& self, const T& id) {
	(void)id;

	if constexpr(std::is_same_v<T, test_static_decorations>) {
		return self.static_decorations;
	}
	else if constexpr(std::is_same_v<T, test_plain_sprited_bodies>) {
		return self.plain_sprited_bodies;
	}
	else if constexpr(std::is_same_v<T, test_dynamic_decorations>) {
		return self.dynamic_decorations;
	}
	else if constexpr(std::is_same_v<T, test_sound_decorations>) {
		return self.sound_decorations;
	}
	else if constexpr(std::is_same_v<T, test_particles_decorations>) {
		return self.particles_decorations;
	}
	else if constexpr(std::is_same_v<T, point_marker_type>) {
		return self.point_markers;
	}
	else if constexpr(std::is_same_v<T, area_marker_type>) {
		return self.area_markers;
	}
	else if constexpr(std::is_same_v<T, test_static_lights>) {
		return self.lights;
	}
	else if constexpr(std::is_same_v<T, test_wandering_pixels_decorations>) {
		return self.wandering_pixels;
	}
	else if constexpr(std::is_same_v<T, test_shootable_weapons>) {
		return self.firearms;
	}
	else if constexpr(std::is_same_v<T, test_container_items>) {
		return self.magazines;
	}
	else if constexpr(std::is_same_v<T, test_shootable_charges>) {
		return self.shootable_charges;
	}
	else if constexpr(std::is_same_v<T, test_tool_items>) {
		return self.tools;
	}
	else if constexpr(std::is_same_v<T, test_melee_weapons>) {
		return self.melees;
	}
	else if constexpr(std::is_same_v<T, test_hand_explosives>) {
		return self.explosives;
	}
	else if constexpr(std::is_same_v<T, test_scene_physical_material_id>) {
		return self.materials;
	}
	else if constexpr(std::is_same_v<T, editor_builtin_prefab_type>) {
		return self.prefabs;
	}
	else {
		static_assert(always_false_v<T>, "Non-exhaustive if constexpr");
	}
}

template <class T>
const auto& editor_official_resource_map::dereference_enum(const T& typed) const {
	return get_container(*this, typed).at(typed);
}

template <class T>
auto& editor_official_resource_map::dereference_enum(const T& typed) {
	return get_container(*this, typed)[typed];
}

template <class V>
auto& editor_official_resource_map::operator[](const V& var) {
	if constexpr(is_variant_v<V>) {
		return std::visit(
			[this]<typename T>(const T& typed) -> auto& {
				return this->dereference_enum(typed);
			},
			var
		);
	}
	else {
		return dereference_enum(var);
	}
}

template <class V>
const auto& editor_official_resource_map::operator[](const V& var) const {
	if constexpr(is_variant_v<V>) {
		return std::visit(
			[this]<typename T>(const T& typed) -> auto& {
				return this->dereference_enum(typed);
			},
			var
		);
	}
	else {
		return dereference_enum(var);
	}
}
