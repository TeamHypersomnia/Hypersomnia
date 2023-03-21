#include <unordered_map>
#include "test_scenes/test_scene_flavour_ids.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "augs/templates/traits/is_variant.h"

struct editor_official_resource_map {
	template <class A, class B>
	using Map = std::unordered_map<A, editor_typed_resource_id<B>>;

	Map<test_static_decorations, editor_sprite_resource> static_decorations;
	Map<test_plain_sprited_bodies, editor_sprite_resource> plain_sprited_bodies;
	Map<test_dynamic_decorations, editor_sprite_resource> dynamic_decorations;
	Map<test_sound_decorations, editor_sound_resource> sound_decorations;
	Map<area_marker_type, editor_area_marker_resource> box_markers;
	Map<test_static_lights, editor_light_resource> lights;
	Map<test_particles_decorations, editor_particles_resource> particles_decorations;
	Map<test_wandering_pixels_decorations, editor_wandering_pixels_resource> wandering_pixels;

	Map<test_shootable_weapons, editor_firearm_resource> firearms;
	Map<test_melee_weapons, editor_melee_resource> melees;

	template <class S, class T>
	static auto& get_container(S& self, const T&) {
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
		else if constexpr(std::is_same_v<T, area_marker_type>) {
			return self.box_markers;
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
		else if constexpr(std::is_same_v<T, test_melee_weapons>) {
			return self.melees;
		}
		else {
			static_assert(always_false_v<T>, "Non-exhaustive if constexpr");
		}
	}

	template <class T>
	const auto& dereference_enum(const T& typed) const {
		return get_container(*this, typed).at(typed);
	}

	template <class T>
	auto& dereference_enum(const T& typed) {
		return get_container(*this, typed)[typed];
	}

	template <class V>
	auto& operator[](const V& var) {
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
	const auto& operator[](const V& var) const {
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
};

