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
#include "game/components/sender_component.h"
#include "game/components/missile_component.h"
#include "game/components/explosive_component.h"

#include "game/components/sound_existence_component.h"
#include "game/components/particles_existence_component.h"

#include "game/detail/physics/physics_scripts.h"
#include "game/detail/inventory/inventory_slot_handle.h"

#include "game/enums/rigid_body_type.h"

std::ostream& operator<<(std::ostream& out, const entity_handle &x) {
	return out << typesafe_sprintf("%x %x", to_string(x.get_name()), x.get_id());
}

std::ostream& operator<<(std::ostream& out, const const_entity_handle &x) {
	return out << typesafe_sprintf("%x %x", to_string(x.get_name()), x.get_id());
}

template <bool C>
template <bool, class>
entity_handle basic_entity_handle<C>::add_standard_components(const logic_step step) const {
	return add_standard_components<true, void>(step, true);
}

template <bool C>
template <bool, class>
entity_handle basic_entity_handle<C>::add_standard_components(const logic_step step, const bool activate_inferred) const {
	if (dead()) {
		return *this;
	}

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
		get<components::interpolation>().place_of_birth = this->get_logic_transform();
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
		add(components::tree_of_npo_node::create_default_for(*this));
	}

	if (has<components::rigid_body>()) {
		if (!has<components::special_physics>()) {
			add(components::special_physics());
		}

		get<components::special_physics>().dropped_or_created_cooldown.set(200, get_cosmos().get_timestamp());
	}

	if (
		has<components::missile>()
		|| has<components::explosive>()
	) {
		ensure(has<components::sender>());
	}

	recalculate_basic_processing_categories<true, void>();
	
	if (activate_inferred) {
		get<components::all_inferred_state>().set_activated(true);
	}

	return *this;
}

template <bool C>
template <bool, class>
void basic_entity_handle<C>::set_name(const entity_name_type& new_name) const {
	get<components::type>().set_name(new_name);
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
template entity_handle basic_entity_handle<false>::add_standard_components<true, void>(const logic_step, const bool) const;
template entity_handle basic_entity_handle<false>::add_standard_components<true, void>(const logic_step) const;
template void basic_entity_handle<false>::set_name<true, void>(const entity_name_type&) const;
template void basic_entity_handle<false>::recalculate_basic_processing_categories<true, void>() const;
