#include "augs/templates/container_templates.h"
#include "augs/templates/introspect.h"
#include "cosmic_entropy.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"

#include "augs/misc/machine_entropy.h"
#include "augs/templates/introspection_utils/rewrite_members.h"

#include "game/detail/inventory/perform_transfer.h"

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
	total += wields_per_entity.size();

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

	for (const auto& wield : b.wields_per_entity) {
		if (wields_per_entity.find(wield.first) == wields_per_entity.end()) {
			wields_per_entity[wield.first] = wield.second;
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
	augs::introspect(
		[&](auto, auto& member_container) {
			using T = remove_cref<decltype(member_container)>;
			
			if constexpr(!std::is_same_v<decltype(transfer_requests), T>) {
				auto eraser = [&cosm](const auto& it) {
					return cosm[it.first].dead();
				};

				erase_if(member_container, eraser);
			}
		},
		*this
	);

	erase_if(transfer_requests, [&](const auto& request) {
		return cosm[request.item].dead();
	});
}

bool guid_mapped_entropy::operator!=(const guid_mapped_entropy& b) const {
	return !(
		intents_per_entity == b.intents_per_entity
		&& motions_per_entity == b.motions_per_entity
		&& wields_per_entity == b.wields_per_entity
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

	const auto& solvable = mapper.get_solvable();

	for (const auto& entry : b.cast_spells_per_entity) {
		cast_spells_per_entity[mapper[entry.first].get_guid()] = entry.second;
	}

	for (const auto& entry : b.wields_per_entity) {
		wields_per_entity[mapper[entry.first].get_guid()] = solvable.guidize(entry.second);
	}

	for (const auto& entry : b.transfer_requests) {
		transfer_requests.push_back(solvable.guidize(entry));
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

	const auto& solvable = mapper.get_solvable();

	for (const auto& entry : b.cast_spells_per_entity) {
		cast_spells_per_entity[mapper[entry.first].get_id()] = entry.second;
	}

	for (const auto& entry : b.wields_per_entity) {
		wields_per_entity[mapper[entry.first].get_id()] = solvable.deguidize(entry.second);
	}

	for (const auto& entry : b.transfer_requests) {
		transfer_requests.push_back(solvable.deguidize(entry));
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
