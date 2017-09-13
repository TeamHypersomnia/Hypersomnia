#pragma once
#include <vector>
#include <unordered_map>

#include "augs/templates/container_templates.h"

#include "augs/misc/streams.h"
#include "augs/misc/machine_entropy.h"
#include "augs/misc/container_with_small_size.h"

#include "augs/window_framework/event.h"

#include "game/transcendental/entity_id.h"
#include "game/enums/game_intent_type.h"
#include "game/messages/intent_message.h"
#include "game/detail/inventory/item_slot_transfer_request.h"
#include "game/components/sentience_component.h"

class cosmos;

template <class key>
struct basic_cosmic_entropy {
	// GEN INTROSPECTOR struct basic_cosmic_entropy class key
	augs::container_with_small_size<std::unordered_map<key, spell_id>, unsigned char> cast_spells_per_entity;
	augs::container_with_small_size<std::unordered_map<key, game_intents>, unsigned char> intents_per_entity;
	augs::container_with_small_size<std::unordered_map<key, game_motions>, unsigned char> motions_per_entity;
	augs::container_with_small_size<std::vector<basic_item_slot_transfer_request<key>>, unsigned short> transfer_requests;
	// END GEN INTROSPECTOR

	void override_transfers_leaving_other_entities(
		const cosmos&,
		std::vector<basic_item_slot_transfer_request<key>> new_transfers
	);

	size_t length() const;

	basic_cosmic_entropy& operator+=(const basic_cosmic_entropy& b);

	void clear();
};

struct cosmic_entropy;

struct guid_mapped_entropy : basic_cosmic_entropy<entity_guid> {
	using base = basic_cosmic_entropy<entity_guid>;
	
	// GEN INTROSPECTOR struct guid_mapped_entropy
	// INTROSPECT BASE basic_cosmic_entropy<entity_guid>
	// END GEN INTROSPECTOR

	guid_mapped_entropy() = default;
	explicit guid_mapped_entropy(const cosmic_entropy&, const cosmos&);
	
	guid_mapped_entropy& operator+=(const guid_mapped_entropy& b) {
		base::operator+=(b);
		return *this;
	}

	bool operator!=(const guid_mapped_entropy&) const;
};

struct cosmic_entropy : basic_cosmic_entropy<entity_id> {
	using base = basic_cosmic_entropy<entity_id>;

	// GEN INTROSPECTOR struct cosmic_entropy
	// INTROSPECT BASE basic_cosmic_entropy<entity_id>
	// END GEN INTROSPECTOR

	cosmic_entropy() = default;
	
	explicit cosmic_entropy(
		const guid_mapped_entropy&, 
		const cosmos&
	);
	
	explicit cosmic_entropy(
		const const_entity_handle controlled_entity, 
		const game_intents&,
		const game_motions&
	);

	cosmic_entropy& operator+=(const cosmic_entropy& b) {
		base::operator+=(b);
		return *this;
	}
};