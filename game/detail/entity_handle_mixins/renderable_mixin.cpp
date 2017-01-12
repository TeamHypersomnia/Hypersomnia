#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "renderable_mixin.h"

#include "game/components/polygon_component.h"
#include "game/components/sprite_component.h"
#include "game/components/tile_layer_instance_component.h"
#include "game/components/particles_existence_component.h"
#include "game/components/transform_component.h"
#include "game/components/substance_component.h"
#include "game/components/wandering_pixels_component.h"
#include "game/components/sound_existence_component.h"

template <bool C, class D>
ltrb basic_renderable_mixin<C, D>::get_aabb(const renderable_positioning_type type) const {
	const auto handle = *static_cast<const D*>(this);

	return get_aabb(handle.logic_transform(), type);
}

template <bool C, class D>
ltrb basic_renderable_mixin<C, D>::get_aabb(const components::transform transform, const renderable_positioning_type positioning) const {
	const auto handle = *static_cast<const D*>(this);

	const auto* const sprite = handle.find<components::sprite>();

	if (sprite) {
		return sprite->get_aabb(transform, positioning);
	}
	
	const auto* const polygon = handle.find<components::polygon>();

	if (polygon) {
		return polygon->get_aabb(transform);
	}
	
	const auto* const tile_layer_instance = handle.find<components::tile_layer_instance>();

	if (tile_layer_instance) {
		return tile_layer_instance->get_aabb(transform);
	}
	
	const auto* const wandering_pixels = handle.find<components::wandering_pixels>();

	if (wandering_pixels) {
		return wandering_pixels->reach;
	}
	
	const auto* const particles_existence = handle.find<components::particles_existence>();

	if (particles_existence) {
		ltrb aabb;
		aabb.set_position(transform.pos);
		aabb.set_size({ 2.f, 2.f });

		const auto enlarge = std::max(particles_existence->input.randomize_position_within_radius, particles_existence->distribute_within_segment_of_length);
		aabb.expand_from_center({ enlarge, enlarge });

		return aabb;
	}

	//const auto* const sound_existence = e.find<components::sound_existence>();
	//if (sound_existence) {
	//	result.type = tree_type::SOUND_EXISTENCES;
	//	result.aabb.set_position(e.logic_transform().pos);
	//
	//	const float artifacts_avoidance_epsilon = 20.f;
	//
	//	const float distance = sound_existence->calculate_max_audible_distance() + artifacts_avoidance_epsilon;
	//	result.aabb.set_size({ distance*2, distance * 2 });
	//}

	ensure(false);

	return{};
}

template <bool C, class D>
ltrb basic_renderable_mixin<C, D>::get_aabb(const interpolation_system& interp, const renderable_positioning_type type) const {
	const auto handle = *static_cast<const D*>(this);
	
	return get_aabb(handle.viewing_transform(interp, true), type);
}

// explicit instantiation
template class renderable_mixin<false, basic_entity_handle<false>>;
template class renderable_mixin<true, basic_entity_handle<true>>;
template class basic_renderable_mixin<false, basic_entity_handle<false>>;
template class basic_renderable_mixin<true, basic_entity_handle<true>>;
