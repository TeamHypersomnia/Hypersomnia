#include "augs/misc/randomization.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "game/components/render_component.h"
#include "game/components/processing_component.h"
#include "game/components/interpolation_component.h"
#include "game/components/crosshair_component.h"
#include "game/components/container_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/sender_component.h"
#include "game/components/missile_component.h"
#include "game/components/explosive_component.h"

#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/view_input/particle_effect_input.h"

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
	if (dead()) {
		return *this;
	}

	if (const auto rigid_body = find<components::rigid_body>()) {
		rigid_body.get_special().dropped_or_created_cooldown.set(200, get_cosmos().get_timestamp());
	}

	if ((has<components::missile>() || find<invariants::explosive>()) && !has<components::sender>()) {
		add(components::sender());
	}

	recalculate_basic_processing_categories<true, void>();
	
	cosmic::infer_caches_for(*this);

	if (const auto interpolation = find<components::interpolation>()) {
		interpolation->place_of_birth = this->get_logic_transform();
	}

	if (const auto trace = find<components::trace>()) {
		auto rng = get_cosmos().get_fast_rng_for(get_id());
		trace->reset(get<invariants::trace>(), rng);
	}

	return *this;
}

template <bool C>
template <bool, class>
void basic_entity_handle<C>::recalculate_basic_processing_categories() const {
	ensure(alive());
	const auto default_processing = components::processing::get_default(*this);

	get<components::processing>().set_basic_categories(default_processing.processing_subject_categories);
}

// explicit instantiation
template entity_handle basic_entity_handle<false>::add_standard_components<true, void>(const logic_step) const;
template void basic_entity_handle<false>::recalculate_basic_processing_categories<true, void>() const;
