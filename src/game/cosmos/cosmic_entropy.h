#pragma once
#include <vector>
#include <unordered_map>

#include "augs/readwrite/memory_stream.h"
#include "augs/misc/machine_entropy.h"
#include "augs/misc/container_with_small_size.h"

#include "augs/window_framework/event.h"

#include "game/cosmos/entity_id.h"
#include "game/enums/game_intent_type.h"
#include "game/messages/intent_message.h"
#include "game/detail/inventory/item_slot_transfer_request.h"
#include "game/detail/spells/all_spells.h"
#include "game/detail/inventory/wielding_setup.h"

class cosmos;

template <class key>
struct basic_cosmic_entropy {
	// GEN INTROSPECTOR struct basic_cosmic_entropy class key
	augs::container_with_small_size<std::unordered_map<key, spell_id>, unsigned char> cast_spells_per_entity;
	augs::container_with_small_size<std::unordered_map<key, basic_wielding_setup<key>>, unsigned char> wields_per_entity;

	augs::container_with_small_size<std::unordered_map<key, game_intents>, unsigned char> intents_per_entity;
	augs::container_with_small_size<std::unordered_map<key, game_motions>, unsigned char> motions_per_entity;
	augs::container_with_small_size<std::vector<basic_item_slot_transfer_request<key>>, unsigned short> transfer_requests;
	// END GEN INTROSPECTOR

	std::size_t length() const;

	basic_cosmic_entropy& operator+=(const basic_cosmic_entropy& b);

	void clear_dead_entities(const cosmos&);
	void clear();

	bool empty() const;
};

struct cosmic_entropy;

struct guid_mapped_entropy : basic_cosmic_entropy<entity_guid> {
	using base = basic_cosmic_entropy<entity_guid>;
	using introspect_base = base;
	
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
	using introspect_base = base;

	cosmic_entropy() = default;
	
	explicit cosmic_entropy(
		const guid_mapped_entropy&, 
		const cosmos&
	);
	
	explicit cosmic_entropy(
		const entity_id controlled_entity,
		const game_intents&,
		const game_motions&
	);

	cosmic_entropy& operator+=(const cosmic_entropy& b) {
		base::operator+=(b);
		return *this;
	}
};