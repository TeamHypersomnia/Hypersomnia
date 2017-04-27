#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "game/components/render_component.h"
#include "game/components/all_inferred_state_component.h"
#include "game/components/processing_component.h"
#include "game/components/tree_of_npo_node_component.h"
#include "game/components/special_physics_component.h"
#include "game/components/interpolation_component.h"
#include "game/components/crosshair_component.h"
#include "game/components/container_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/position_copying_component.h"

#include "game/components/sound_existence_component.h"
#include "game/components/particles_existence_component.h"

//#include "game/systems_audiovisual/interpolation_system.h"

#include "game/detail/physics/physics_scripts.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "game/detail/gui/gui_positioning.h"

#include "game/enums/rigid_body_type.h"

std::ostream& operator<<(std::ostream& out, const entity_handle &x) {
	return out << typesafe_sprintf("%x %x", x.get_debug_name(), x.get_id());
}

std::ostream& operator<<(std::ostream& out, const const_entity_handle &x) {
	return out << typesafe_sprintf("%x %x", x.get_debug_name(), x.get_id());
}

template <bool C>
template <bool, class>
basic_entity_handle<C> basic_entity_handle<C>::add_standard_components(const logic_step step) const {
	ensure(alive());
	const bool has_physics = has<components::rigid_body>();

	if (has_physics || has<components::fixtures>()) {
		const bool has_transform = has<components::transform>();

		ensure(!has_transform);
	}

	if (!has<components::interpolation>() 
		&& (
			(has_physics && get<components::rigid_body>().get_body_type() != rigid_body_type::STATIC)
			|| has<components::crosshair>() 
			|| has<components::position_copying>()
		)
	) {
		add(components::interpolation());
		get<components::interpolation>().place_of_birth = get_logic_transform();
	}

	if ((
		has<components::render>() 
		|| has<components::particles_existence>()
		//|| has<components::sound_existence>()
		)
		&& !has<components::fixtures>() 
		&& !has<components::rigid_body>() 
		&& !has<components::tree_of_npo_node>()
	) {
		add(components::tree_of_npo_node::create_default_for(step, *this));
	}

	if (has<components::rigid_body>()) {
		if (!has<components::special_physics>()) {
			add(components::special_physics());
		}

		get<components::special_physics>().dropped_or_created_cooldown.set(200, get_cosmos().get_timestamp());
	}

	recalculate_basic_processing_categories<false, void>();
	
	if (!has<components::all_inferred_state>()) {
		add(components::all_inferred_state());
	}

	return *this;
}

template <bool C>
template <bool, class>
void basic_entity_handle<C>::recalculate_basic_processing_categories() const {
	ensure(alive());
	const auto default_processing = components::processing::get_default(*this);

	if (!has<components::processing>()) {
		add(default_processing);
	}
	else {
		get<components::processing>().set_basic_categories(default_processing.processing_subject_categories);
	}
}

// explicit instantiation
template basic_entity_handle<false> basic_entity_handle<false>::add_standard_components<false, void>(const logic_step) const;
template void basic_entity_handle<false>::recalculate_basic_processing_categories<false, void>() const;
