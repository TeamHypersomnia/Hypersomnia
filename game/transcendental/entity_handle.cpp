#include "entity_handle.h"
#include "game/components/render_component.h"
#include "game/components/substance_component.h"
#include "game/components/processing_component.h"
#include "game/components/dynamic_tree_node_component.h"
#include "game/components/special_physics_component.h"
#include "game/components/interpolation_component.h"
#include "game/components/crosshair_component.h"
#include "game/components/container_component.h"
#include "game/components/gui_element_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/position_copying_component.h"

#include "game/components/sound_existence_component.h"
#include "game/components/particles_existence_component.h"

//#include "game/systems_audiovisual/interpolation_system.h"

#include "game/transcendental/cosmos.h"
#include "game/detail/physics_scripts.h"
#include "game/detail/inventory_slot_handle.h"
#include "game/detail/gui/gui_positioning.h"

template <bool C>
template <bool, class>
void basic_entity_handle<C>::add_standard_components() const {
	ensure(alive());
	const bool has_physics = has<components::physics>();

	if (has_physics || has<components::fixtures>()) {
		const bool has_transform = has<components::transform>();

		ensure(!has_transform);
	}

	if (!has<components::interpolation>() && 
		(has_physics || has<components::crosshair>() || has<components::position_copying>())
		) {
		add(components::interpolation());
		get<components::interpolation>().place_of_birth = logic_transform();
	}

	if ((has<components::render>() 
		|| has<components::particles_existence>()
		//|| has<components::sound_existence>()
		)
		&& !is_entity_physical(*this) && !has<components::dynamic_tree_node>()) {
		add(components::dynamic_tree_node::get_default(*this));
	}

	if (has<components::physics>()) {
		if (!has<components::special_physics>()) {
			add(components::special_physics());
		}

		get<components::special_physics>().since_dropped.set(200, get_cosmos().get_timestamp());
	}

	recalculate_basic_processing_categories<false, void>();
	
	if (!has<components::substance>()) {
		add(components::substance());
	}

	if (has<components::gui_element>()) {
		const auto& container = get<components::container>();

		for (const auto& s : container.slots) {
			initialize_slot_button_for_new_gui_owner(get_cosmos()[inventory_slot_id(s.first, get_id())]);
		}
	}
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
template void basic_entity_handle<false>::add_standard_components<false, void>() const;
template void basic_entity_handle<false>::recalculate_basic_processing_categories<false, void>() const;
