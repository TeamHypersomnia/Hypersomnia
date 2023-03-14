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

	template <class T>
	auto& dereference_enum(const T& typed) {
		if constexpr(std::is_same_v<T, test_static_decorations>) {
			return static_decorations[typed];
		}
		else if constexpr(std::is_same_v<T, test_plain_sprited_bodies>) {
			return plain_sprited_bodies[typed];
		}
		else if constexpr(std::is_same_v<T, test_dynamic_decorations>) {
			return dynamic_decorations[typed];
		}
		else if constexpr(std::is_same_v<T, test_sound_decorations>) {
			return sound_decorations[typed];
		}
		else if constexpr(std::is_same_v<T, test_particles_decorations>) {
			return particles_decorations[typed];
		}
		else if constexpr(std::is_same_v<T, area_marker_type>) {
			return box_markers[typed];
		}
		else if constexpr(std::is_same_v<T, test_static_lights>) {
			return lights[typed];
		}
		else if constexpr(std::is_same_v<T, test_wandering_pixels_decorations>) {
			return wandering_pixels[typed];
		}
		else {
			static_assert(always_false_v<T>, "Non-exhaustive if constexpr");
		}
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
};

