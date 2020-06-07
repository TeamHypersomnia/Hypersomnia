#pragma once
#include <array>
#include "game/cosmos/entity_id.h"
#include "game/detail/inventory/wielding_setup.h"

struct akimbo_reload_state {
	// GEN INTROSPECTOR struct akimbo_reload_state
	signi_entity_id next;
	signi_wielding_setup wield_on_complete;
	// END GEN INTROSPECTOR

	bool is_set() const {
		return next.is_set() || wield_on_complete.is_set();
	}
};

struct reloading_context {
	// GEN INTROSPECTOR struct reloading_context
	signi_inventory_slot_id concerned_slot;
	signi_entity_id new_ammo_source;
	signi_entity_id old_ammo_source;
	// END GEN INTROSPECTOR

	template <class C>
	bool alive(const C& cosm) const {
		return cosm[concerned_slot].alive();
	}

	bool significantly_different_from(const reloading_context& b) const;
	bool is_chambering() const;
};
