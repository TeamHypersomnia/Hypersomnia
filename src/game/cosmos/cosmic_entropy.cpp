#include "augs/templates/container_templates.h"
#include "augs/templates/introspect.h"
#include "cosmic_entropy.h"
#include "game/cosmos/cosmos.h"

#include "augs/misc/machine_entropy.h"
#include "augs/templates/introspection_utils/rewrite_members.h"

#include "game/detail/inventory/perform_transfer.h"

#if TODO
template <class key>
void basic_cosmic_entropy<key>::override_transfers_leaving_other_entities(
	const cosmos& cosm,
	std::vector<basic_item_slot_transfer_request<key>> new_transfers
) {
	erase_if(transfer_requests, [&](const basic_item_slot_transfer_request<key> o) {
		const auto overridden_transfer = match_transfer_capabilities(cosm, cosm.get_solvable().deguidize(o));
		
		ensure(overridden_transfer.is_legal());

		for (const auto n : new_transfers) {
			const auto new_transfer = cosm.get_solvable().deguidize(n);
			
			if (match_transfer_capabilities(cosm, new_transfer).authorized_capability
				== overridden_transfer.authorized_capability
			) {
				return true;
			}
		}

		return false;
	});

	concatenate(transfer_requests, new_transfers);
}
#endif

template <class key>
size_t basic_cosmic_entropy<key>::length() const {
	size_t total = 0;

	for (const auto& ent : intents_per_entity) {
		total += ent.second.size();
	}

	for (const auto& ent : motions_per_entity) {
		total += ent.second.size();
	}

	total += transfer_requests.size();
	total += cast_spells_per_entity.size();

	return total;
}

template <class key>
basic_cosmic_entropy<key>& basic_cosmic_entropy<key>::operator+=(const basic_cosmic_entropy& b) {
	for (const auto& intents : b.intents_per_entity) {
		concatenate(intents_per_entity[intents.first], intents.second);
	}

	for (const auto& intents : b.motions_per_entity) {
		concatenate(motions_per_entity[intents.first], intents.second);
	}

	concatenate(transfer_requests, b.transfer_requests);

	for (const auto& spell : b.cast_spells_per_entity) {
		if (cast_spells_per_entity.find(spell.first) == cast_spells_per_entity.end()) {
			cast_spells_per_entity[spell.first] = spell.second;
		}
	}

	return *this;
}

template <class key>
void basic_cosmic_entropy<key>::clear() {
	augs::introspect(
		[](auto, auto& member) {
			member.clear();
		},
		*this
	);
}

template <class key>
void basic_cosmic_entropy<key>::clear_dead_entities(const cosmos& cosm) {
	auto eraser = [&cosm](const auto& it) {
		return cosm[it.first].dead();
	};

	augs::introspect(
		[&](auto, auto& member_container) {
			using T = remove_cref<decltype(member_container)>;
			
			if constexpr(!std::is_same_v<decltype(transfer_requests), T>) {
				erase_if(member_container, eraser);
			}
		},
		*this
	);
}

bool guid_mapped_entropy::operator!=(const guid_mapped_entropy& b) const {
	return !(
		intents_per_entity == b.intents_per_entity
		&& motions_per_entity == b.motions_per_entity
		&& cast_spells_per_entity == b.cast_spells_per_entity
		&& transfer_requests == b.transfer_requests
	);
}

guid_mapped_entropy::guid_mapped_entropy(
	const cosmic_entropy& b,
	const cosmos& mapper
) {
	for (const auto& entry : b.intents_per_entity) {
		intents_per_entity[mapper[entry.first].get_guid()] = entry.second;
	}

	for (const auto& entry : b.motions_per_entity) {
		motions_per_entity[mapper[entry.first].get_guid()] = entry.second;
	}

	for (const auto& entry : b.cast_spells_per_entity) {
		cast_spells_per_entity[mapper[entry.first].get_guid()] = entry.second;
	}

	for (const auto& entry : b.transfer_requests) {
		transfer_requests.push_back(mapper.get_solvable().guidize(entry));
	}
}

cosmic_entropy::cosmic_entropy(
	const guid_mapped_entropy& b, 
	const cosmos& mapper
) {
	for (const auto& entry : b.intents_per_entity) {
		intents_per_entity[mapper[entry.first].get_id()] = entry.second;
	}

	for (const auto& entry : b.motions_per_entity) {
		motions_per_entity[mapper[entry.first].get_id()] = entry.second;
	}

	for (const auto& entry : b.cast_spells_per_entity) {
		cast_spells_per_entity[mapper[entry.first].get_id()] = entry.second;
	}

	for (const auto& entry : b.transfer_requests) {
		transfer_requests.push_back(mapper.get_solvable().deguidize(entry));
	}
}

cosmic_entropy::cosmic_entropy(
	const entity_id controlled_entity,
	const game_intents& intents,
	const game_motions& motions
) {
	intents_per_entity[controlled_entity] = intents;
	motions_per_entity[controlled_entity] = motions;
}

template struct basic_cosmic_entropy<entity_id>;
template struct basic_cosmic_entropy<entity_guid>;
