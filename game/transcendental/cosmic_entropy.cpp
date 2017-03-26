#include "cosmic_entropy.h"
#include "game/transcendental/cosmos.h"

#include "augs/misc/machine_entropy.h"
#include "augs/misc/input_context.h"

#include "game/detail/inventory/inventory_utils.h"
#include "generated_introspectors.h"

template <class key>
void basic_cosmic_entropy<key>::override_transfers_leaving_other_entities(
	const cosmos& cosm,
	std::vector<basic_item_slot_transfer_request_data<key>> new_transfers
) {
	erase_remove(transfer_requests, [&](const basic_item_slot_transfer_request_data<key> o) {
		const auto overridden_transfer = match_transfer_capabilities(cosm, cosm.deguidize(o));
		
		ensure(overridden_transfer.is_legal());

		for (const auto n : new_transfers) {
			const auto new_transfer = cosm[cosm.deguidize(n)];
			
			if (
				match_transfer_capabilities(cosm, new_transfer).authorized_capability
				== overridden_transfer.authorized_capability
			) {
				return true;
			}
		}

		return false;
	});

	concatenate(transfer_requests, new_transfers);
}

template <class key>
size_t basic_cosmic_entropy<key>::length() const {
	size_t total = 0;

	for (const auto& ent : intents_per_entity) {
		total += ent.second.size();
	}

	total += transfer_requests.size();
	total += cast_spells.size();

	return total;
}

template <class key>
basic_cosmic_entropy<key>& basic_cosmic_entropy<key>::operator+=(const basic_cosmic_entropy& b) {
	for (const auto& intents : b.intents_per_entity) {
		concatenate(intents_per_entity[intents.first], intents.second);
	}

	concatenate(transfer_requests, b.transfer_requests);

	for (const auto& spell : b.cast_spells) {
		if (cast_spells.find(spell.first) == cast_spells.end()) {
			cast_spells[spell.first] = spell.second;
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

bool guid_mapped_entropy::operator!=(const guid_mapped_entropy& b) const {
	return !(
		compare_containers(intents_per_entity, b.intents_per_entity)
		&& compare_containers(cast_spells, b.cast_spells)
		&& compare_containers(transfer_requests, b.transfer_requests)
	);
}

guid_mapped_entropy::guid_mapped_entropy(
	const cosmic_entropy& b,
	const cosmos& mapper
) {
	for (const auto& entry : b.intents_per_entity) {
		intents_per_entity[mapper[entry.first].get_guid()] = entry.second;
	}

	for (const auto& entry : b.cast_spells) {
		cast_spells[mapper[entry.first].get_guid()] = entry.second;
	}

	for (const auto& entry : b.transfer_requests) {
		transfer_requests.push_back(mapper.guidize(entry));
	}
}

cosmic_entropy::cosmic_entropy(
	const guid_mapped_entropy& b, 
	const cosmos& mapper
) {
	for (const auto& entry : b.intents_per_entity) {
		intents_per_entity[mapper.get_handle(entry.first).get_id()] = entry.second;
	}

	for (const auto& entry : b.cast_spells) {
		cast_spells[mapper.get_handle(entry.first).get_id()] = entry.second;
	}

	for (const auto& entry : b.transfer_requests) {
		transfer_requests.push_back(mapper.deguidize(entry));
	}
}

cosmic_entropy::cosmic_entropy(
	const const_entity_handle controlled_entity,
	const decltype(intents_per_entity[entity_id()])& intents
) {
	intents_per_entity[controlled_entity] = intents;
}

template struct basic_cosmic_entropy<entity_id>;
template struct basic_cosmic_entropy<entity_guid>;
