#include "augs/templates/logically_empty.h"
#include "augs/templates/container_templates.h"
#include "cosmic_entropy.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"

#include "augs/misc/machine_entropy.h"
#include "game/cosmos/entropy_recording_options.h"


template <class K>
std::size_t basic_player_commands<K>::length() const {
	std::size_t total = 0;

	total += motions.size();
	total += intents.size();

	if (logically_set(transfer)) {
		++total;
	}

	if (logically_set(cast_spell)) {
		++total;
	}

	if (logically_set(wield)) { 
		++total;
	}

	return total;
}

template <class K>
bool basic_player_commands<K>::empty() const {
	return length() == 0;
}

template <class key>
std::size_t basic_cosmic_entropy<key>::length() const {
	std::size_t total = 0;

	for (const auto& p : players) {
		total += p.second.commands.length();
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
basic_player_commands<K>& basic_player_commands<K>::operator+=(const basic_player_commands<K>& r) {
	concatenate(intents, r.intents);
	
	for (const auto it : r.motions) {
		if (auto m = mapped_or_nullptr(motions, it.first)) {
			*m += it.second;
		}
		else {
			motions.emplace(it.first, it.second);
		}
	}

	if (logically_set(r.transfer)) {
		transfer = r.transfer;
	}

	if (logically_set(r.wield)) {
		wield = r.wield;
	}

	if (logically_set(r.cast_spell)) {
		cast_spell = r.cast_spell;
	}

	return *this;
}

template <class K>
bool basic_player_commands<K>::operator==(const basic_player_commands<K>& b) const {
	return
		intents == b.intents
		&& motions == b.motions
		&& wield == b.wield
		&& cast_spell == b.cast_spell
		&& transfer == b.transfer
	;
}

template <class K>
bool basic_player_commands<K>::operator!=(const basic_player_commands<K>& b) const {
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
void basic_player_commands<K>::clear() {
	clear_relevant({});
}

template <class K>
void basic_player_commands<K>::clear_relevant(const cosmic_entropy_recording_options opts) {
	if (opts.overwrite_intents) {
		intents.clear();
	}

	if (opts.overwrite_motions) {
		motions.clear();
	}

	if (opts.overwrite_rest) {
		cast_spell.unset();
		wield = {};
		transfer = {};
	}
}

cosmic_entropy::cosmic_entropy(
	const per_character_input_settings& settings,
	const entity_id controlled_entity,
	const game_intents& intents,
	const raw_game_motion_map& motions
) {
	auto& p = players[controlled_entity];
	p.settings = settings;
	p.commands.intents = intents;
	p.commands.motions = motions;
}

template struct basic_cosmic_entropy<entity_id>;
template struct basic_player_commands<entity_id>;
