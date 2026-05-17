#pragma once
#include "game/cosmos/entity_id.h"
#include "game/components/sender_component.h"
#include "game/cosmos/entity_flavour_id.h"
#include "game/detail/spells/spell_id.h"

using cause_sender = components::sender;

struct damage_cause {
	damage_cause() = default;
	damage_cause(const const_entity_handle& handle);

	// GEN INTROSPECTOR struct damage_cause
	entity_flavour_id flavour;
	signi_entity_id entity;

	spell_id spell;
	// END GEN INTROSPECTOR

	bool is_humiliating(const cosmos& cosm) const;
};

struct damage_circumstances {
	// GEN INTROSPECTOR struct damage_circumstances
	bool headshot = false;
	bool wallbang = false;
	pad_bytes<2> pad;
	// END GEN INTROSPECTOR
};

struct damage_origin {
	damage_origin() = default;
	damage_origin(const const_entity_handle& causing_handle);

	// GEN INTROSPECTOR struct damage_origin
	damage_cause cause;
	cause_sender sender;
	damage_circumstances circumstances;
	// END GEN INTROSPECTOR

	template <class E>
	void copy_sender_from(const E& causing_handle);

	template <class E>
	auto get_guilty_of_damaging(const E& victim_handle) const;

	template <class C, class F>
	auto on_tool_used(C& cosm, F callback) const;
};
