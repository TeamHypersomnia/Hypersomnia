#include "augs/templates/container_templates.h"
#include "augs/templates/introspect.h"
#include "cosmic_entropy.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"

#include "augs/misc/machine_entropy.h"
#include "augs/templates/introspection_utils/rewrite_members.h"

#include "game/detail/inventory/perform_transfer.h"
#include "game/cosmos/entropy_recording_options.h"


template <class K>
std::size_t basic_player_entropy<K>::length() const {
	std::size_t total = 0;

	total += motions.size();
	total += intents.size();

	if (transfer.is_set()) {
		++total;
	}

	if (cast_spell.is_set()) {
		++total;
	}

	if (wield != std::nullopt) { 
		++total;
	}

	return total;
}

template <class K>
bool basic_player_entropy<K>::empty() const {
	return length() == 0;
}

template <class key>
std::size_t basic_cosmic_entropy<key>::length() const {
	std::size_t total = 0;

	for (const auto& p : players) {
		total += p.second.length();
	}

	return total;
}

template <class key>
bool basic_cosmic_entropy<key>::empty() const {
	return players.empty() || length() == 0;
}

template <class key>
basic_cosmic_entropy<key>& basic_cosmic_entropy<key>::operator+=(const basic_cosmic_entropy& b) {
	for (const auto& p : b.players) {
		players[p.first] += p.second;
	}

	return *this;
}

template <class key>
void basic_cosmic_entropy<key>::clear() {
	players.clear();
}

template <class key>
void basic_cosmic_entropy<key>::clear_dead_entities(const cosmos& cosm) {
	erase_if(players, [&](auto& p) {
		if (cosm[p.first].dead()) {
			return true;
		}

		return false;
	});
}

template <class K>
basic_player_entropy<K>& basic_player_entropy<K>::operator+=(const basic_player_entropy<K>& r) {
	concatenate(intents, r.intents);
	
	for (const auto& it : r.motions) {
		if (auto m = mapped_or_nullptr(motions, it.first)) {
			*m += it.second;
		}
		else {
			motions.emplace(it.first, it.second);
		}
	}

	if (r.transfer.is_set()) {
		transfer = r.transfer;
	}

	if (r.wield != std::nullopt) {
		wield = r.wield;
	}

	if (r.cast_spell.is_set()) {
		cast_spell = r.cast_spell;
	}

	return *this;
}

template <class K>
bool basic_player_entropy<K>::operator==(const basic_player_entropy<K>& b) const {
	return
		intents == b.intents
		&& motions == b.motions
		&& wield == b.wield
		&& cast_spell == b.cast_spell
		&& transfer == b.transfer
	;
}

template <class K>
bool basic_player_entropy<K>::operator!=(const basic_player_entropy<K>& b) const {
	return !operator==(b);
}

template <class K>
bool basic_cosmic_entropy<K>::operator==(const basic_cosmic_entropy<K>& b) const {
	return players == b.players;
}

template <class K>
bool basic_cosmic_entropy<K>::operator!=(const basic_cosmic_entropy<K>& b) const {
	return !operator==(b);
}

template <class K>
void basic_player_entropy<K>::clear() {
	clear_relevant({});
}

template <class K>
void basic_player_entropy<K>::clear_relevant(const cosmic_entropy_recording_options opts) {
	if (opts.overwrite_intents) {
		intents.clear();
	}

	if (opts.overwrite_motions) {
		motions.clear();
	}

	if (opts.overwrite_rest) {
		cast_spell.unset();
		wield = std::nullopt;
		transfer = {};
	}
}

cosmic_entropy::cosmic_entropy(
	const entity_id controlled_entity,
	const game_intents& intents,
	const game_motions& motions
) {
	auto& p = players[controlled_entity];
	p.intents = intents;
	p.motions = motions;
}

template struct basic_cosmic_entropy<entity_id>;
template struct basic_player_entropy<entity_id>;
