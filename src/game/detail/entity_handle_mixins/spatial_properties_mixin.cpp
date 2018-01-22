#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "game/components/rigid_body_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/wandering_pixels_component.h"
#include "game/components/position_copying_component.h"
#include "game/components/interpolation_component.h"
#include "game/components/item_component.h"

#include "game/components/transform_component.h"
#include "game/components/all_inferred_state_component.h"

#include "spatial_properties_mixin.h"
#include "augs/drawing/drawing.h"

template <bool C, class D>
components::transform basic_spatial_properties_mixin<C, D>::get_logic_transform() const {
	const auto t = find_logic_transform();
	ensure(t.has_value());
	return *find_logic_transform();
}

template <bool C, class D>
std::optional<components::transform> basic_spatial_properties_mixin<C, D>::find_logic_transform() const {
	const auto handle = *static_cast<const D*>(this);

	/*
		Since an alive owner body always implies that the entity has fixtures component,
		it is equivalent to the call of:

		const auto fixtures = owner.template find_def<definitions::fixtures>()

		But we would anyway need to get the owner body so we do it this way.
	*/

	if (const auto owner = handle.get_owner_of_colliders();
		owner.alive()
	) {
		const auto offset = handle.calculate_owner_of_colliders();

		/*
			TODO: retrieve the offset from fixtures cache instead of recalculating it.
		*/

		ensure_eq(owner, offset.owner);
		const auto body_transform = owner.template get<components::rigid_body>().get_transform();

		auto displacement = offset.shape_offset;

		if (!displacement.pos.is_zero()) {
			displacement.pos.rotate(body_transform.rotation, vec2(0, 0));
		}

		return body_transform + displacement;
	}

	/*
		We only ever ask for transforms of fixtures, not rigid bodies. 
		We deem it pointless to ask for a position of a rigid body without fixtures.
		We will always assume that a potential rigid body comes with a fixtures component.	
		Otherwise there is anyway pretty much no reasonable way to simulate a fixtureless body.

		Thus let's have this commented out.

		if (const auto rigid_body = owner.template find<components::rigid_body>()) {
			ensure(false);

			// Should only ever happen for (entities without fixtures, but with rigid bodies), which is almost never

			return owner.template get<components::rigid_body>().get_transform();
		}
	*/

	/* The owner might have been dead due to the item being in a backpack, for example */
	if (const auto item = handle.template find<components::item>()) {
		const auto& cosmos = handle.get_cosmos();
		return cosmos[item->get_current_slot()].get_container().find_logic_transform();
	}

	if (const auto transform = handle.template find<components::transform>()) {
		return *transform;
	}	

	if (const auto wandering_pixels = handle.template find<components::wandering_pixels>()) {
		return wandering_pixels->reach.center();
	}

	return std::nullopt;
}

template <bool C, class D>
vec2 basic_spatial_properties_mixin<C, D>::get_effective_velocity() const {
	const auto handle = *static_cast<const D*>(this);
	const auto owner = handle.get_owner_of_colliders();

	if (owner.alive()) {
		return owner.template get<components::rigid_body>().get_velocity();
	}
	else if (handle.template has<components::position_copying>()) {
		ensure(handle.template has<components::transform>());
		return 
			(handle.template get<components::transform>().pos - handle.template get<components::position_copying>().get_previous_transform().pos)
			/ static_cast<float>(handle.get_cosmos().get_fixed_delta().in_seconds());
	}
	
	return{};
}

template <class D>
void spatial_properties_mixin<false, D>::set_logic_transform(
	const logic_step step,
	const components::transform t
) const {
	const auto handle = *static_cast<const D*>(this);
	const auto owner_body = handle.get_owner_of_colliders();

	const bool this_entity_does_not_have_its_own_transform = 
		owner_body.alive() 
		&& owner_body != handle
	;

	ensure(!this_entity_does_not_have_its_own_transform);
	
	if (const auto rigid_body = handle.template find<components::rigid_body>()) {
		ensure(!handle.template has<components::transform>());
		rigid_body.set_transform(t);
	}
	else {
		handle.template get<components::transform>() = t;

		// TODO: reinfer the npo cache where necessary
	}
}

// explicit instantiation
template class spatial_properties_mixin<false, basic_entity_handle<false>>;
template class spatial_properties_mixin<true, basic_entity_handle<true>>;
template class basic_spatial_properties_mixin<false, basic_entity_handle<false>>;
template class basic_spatial_properties_mixin<true, basic_entity_handle<true>>;