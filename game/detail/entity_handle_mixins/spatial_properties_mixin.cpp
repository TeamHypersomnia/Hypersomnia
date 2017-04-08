#include "game/transcendental/entity_handle.h"

#include "game/components/rigid_body_component.h"
#include "game/components/special_physics_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/wandering_pixels_component.h"
#include "game/components/position_copying_component.h"
#include "game/components/interpolation_component.h"
#include "game/components/item_component.h"

#include "game/components/polygon_component.h"
#include "game/components/sprite_component.h"
#include "game/components/tile_layer_instance_component.h"
#include "game/components/particles_existence_component.h"
#include "game/components/transform_component.h"
#include "game/components/inferred_state_component.h"
#include "game/components/wandering_pixels_component.h"
#include "game/components/sound_existence_component.h"

#include "game/transcendental/cosmos.h"
#include "spatial_properties_mixin.h"
#include "game/systems_audiovisual/interpolation_system.h"

template <bool C, class D>
bool basic_spatial_properties_mixin<C, D>::has_logic_transform() const {
	const auto handle = *static_cast<const D*>(this);
	const auto owner = handle.get_owner_body();
	
	if (owner.alive() && owner != handle) {
		return true;
	}
	else if (handle.has<components::rigid_body>()) {
		return true;
	}
	else if (handle.has<components::transform>()) {
		return true;
	}
	else if (handle.has<components::wandering_pixels>()) {
		return true;
	}

	return false;
}

template <bool C, class D>
components::transform basic_spatial_properties_mixin<C, D>::get_logic_transform() const {
	const auto handle = *static_cast<const D*>(this);

	const auto owner = handle.get_owner_body();

	if (owner.alive()) {
		ensure(!handle.has<components::transform>());

		const auto& phys = owner.get<components::rigid_body>();

		if (owner != handle) {
			const auto& fixtures = handle.get<components::fixtures>();

			if (fixtures.is_activated() && phys.is_activated()) {
				return components::fixtures::transform_around_body(handle, owner.get_logic_transform());
			}
			else {
				ensure(handle.has<components::item>());

				return handle.get_current_slot().get_container().get_logic_transform();
			}
		}
		else {
			if (phys.is_activated()) {
				return{ phys.get_position(), phys.get_angle() };
			}
			else {
				ensure(handle.has<components::item>());

				return handle.get_current_slot().get_container().get_logic_transform();
			}
		}
	}
	else if (handle.has<components::wandering_pixels>()) {
		return handle.get<components::wandering_pixels>().reach.center();
	}
	else {
		return handle.get<components::transform>();
	}	
}

template <bool C, class D>
vec2 basic_spatial_properties_mixin<C, D>::get_effective_velocity() const {
	const auto handle = *static_cast<const D*>(this);
	const auto owner = handle.get_owner_body();

	if (owner.alive()) {
		return owner.get<components::rigid_body>().velocity();
	}
	else if (handle.has<components::position_copying>()) {
		ensure(handle.has<components::transform>());
		return 
			(handle.get<components::transform>().pos - handle.get<components::position_copying>().get_previous_transform().pos)
			/ static_cast<float>(handle.get_cosmos().get_fixed_delta().in_seconds());
	}
	
	return{};
}

template <bool C, class D>
components::transform basic_spatial_properties_mixin<C, D>::get_viewing_transform(const interpolation_system& sys, const bool integerize) const {
	const auto handle = *static_cast<const D*>(this);

	const auto& owner = handle.get_owner_body();

	if (owner.alive() && owner.has<components::interpolation>() && owner != handle) {
		auto in = sys.get_interpolated(owner);

		if (integerize)
			in.pos = vec2i(in.pos);

		return components::fixtures::transform_around_body(handle, in);
	}
	else if (handle.has<components::interpolation>()) {
		return sys.get_interpolated(handle);
	}

	return handle.get_logic_transform();
}


template <bool C, class D>
ltrb basic_spatial_properties_mixin<C, D>::get_aabb() const {
	const auto handle = *static_cast<const D*>(this);

	return get_aabb(handle.get_logic_transform());
}

template <bool C, class D>
ltrb basic_spatial_properties_mixin<C, D>::get_aabb(const interpolation_system& interp) const {
	const auto handle = *static_cast<const D*>(this);

	return get_aabb(handle.get_viewing_transform(interp, true));
}

template <bool C, class D>
ltrb basic_spatial_properties_mixin<C, D>::get_aabb(const components::transform transform) const {
	const auto handle = *static_cast<const D*>(this);

	const auto* const sprite = handle.find<components::sprite>();

	if (sprite) {
		return sprite->get_aabb(transform);
	}

	const auto* const polygon = handle.find<components::polygon>();

	if (polygon) {
		return polygon->get_aabb(transform);
	}

	const auto* const tile_layer_instance = handle.find<components::tile_layer_instance>();

	if (tile_layer_instance) {
		return tile_layer_instance->get_aabb(
			handle.get_cosmos(),
			transform
		);
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

		const auto enlarge = std::max(
			particles_existence->input.displace_source_position_within_radius, 
			particles_existence->distribute_within_segment_of_length
		);

		aabb.expand_from_center({ enlarge, enlarge });

		return aabb;
	}

	//const auto* const sound_existence = e.find<components::sound_existence>();
	//if (sound_existence) {
	//	result.type = tree_type::SOUND_EXISTENCES;
	//	result.aabb.set_position(e.get_logic_transform().pos);
	//
	//	const float artifacts_avoidance_epsilon = 20.f;
	//
	//	const float distance = sound_existence->calculate_max_audible_distance() + artifacts_avoidance_epsilon;
	//	result.aabb.set_size({ distance*2, distance * 2 });
	//}

	//ensure(false);

	/* TODO: Implement get_aabb for physical entities */
	ensure(!handle.has<components::rigid_body>());
	ensure(!handle.has<components::fixtures>());

	return{};
}

template <class D>
void spatial_properties_mixin<false, D>::set_logic_transform(const components::transform t) const {
	if (get_logic_transform() == t) {
		return;
	}
	
	const auto handle = *static_cast<const D*>(this);
	const auto owner = handle.get_owner_body();

	const bool is_only_fixtural = owner.alive() && owner != handle;

	ensure(!is_only_fixtural);
	
	if (handle.has<components::rigid_body>()) {
		ensure(!handle.has<components::transform>());
		const auto& phys = handle.get<components::rigid_body>();
		phys.set_transform(t);
	}
	else {
		handle.get<components::transform>() = t;

		if (handle.has<components::dynamic_tree_node>()) {
			handle.get<components::dynamic_tree_node>().update_proxy();
		}
	}
}

// explicit instantiation
template class spatial_properties_mixin<false, basic_entity_handle<false>>;
template class spatial_properties_mixin<true, basic_entity_handle<true>>;
template class basic_spatial_properties_mixin<false, basic_entity_handle<false>>;
template class basic_spatial_properties_mixin<true, basic_entity_handle<true>>;
