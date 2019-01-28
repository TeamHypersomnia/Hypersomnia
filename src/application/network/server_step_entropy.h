#pragma once
#include "augs/templates/logically_empty.h"
#include "game/modes/mode_entropy.h"

using server_step_entropy = mode_entropy;

struct prestep_client_context {
	// GEN INTROSPECTOR struct prestep_client_context
	uint8_t num_entropies_accepted = 1;
	// END GEN INTROSPECTOR

	bool operator==(const prestep_client_context& b) const {
		return num_entropies_accepted == b.num_entropies_accepted;
	}
};

struct client_entropy_entry {
	// GEN INTROSPECTOR struct client_entropy_entry
	mode_player_id player_id;
	total_mode_player_entropy total;
	// END GEN INTROSPECTOR

	bool operator==(const client_entropy_entry& b) const {
		return player_id == b.player_id && total == b.total;
	}
};

struct compact_server_step_entropy {
	// GEN INTROSPECTOR struct compact_server_step_entropy
	std::vector<client_entropy_entry> players;
	mode_entropy_general general;
	// END GEN INTROSPECTOR

	bool operator==(const compact_server_step_entropy& b) const {
		return players == b.players && general == b.general;
	}

	void operator+=(const client_entropy_entry& e) {
		if (!e.player_id.is_set()) {
			return;
		}

		for (auto& c : players) {
			if (c.player_id == e.player_id) {
				c.total += e.total;
				return;
			}
		}
		
		players.push_back(e);
	}

	template <class F>
	auto unpack(F&& mode_id_to_entity_id) const {
		server_step_entropy out;
		out.general = general;

		for (const auto& p : players) {
			const auto& t = p.total;

			if (logically_set(t.mode)) {
				out.players[p.player_id] = t.mode;
			}

			if (logically_set(t.cosmic)) {
				const auto id = mode_id_to_entity_id(p.player_id);

				if (logically_set(id)) {
					out.cosmic[id] = t.cosmic;
				}
			}
		}

		return out;
	}

	void clear() {
		players.clear();
		general.clear();
	}
};

struct server_step_entropy_meta {
	static constexpr bool force_read_field_by_field = true;

	// GEN INTROSPECTOR struct server_step_entropy_meta
	std::optional<uint32_t> state_hash;
	bool reinference_required = false;
	// END GEN INTROSPECTOR

	bool operator==(const server_step_entropy_meta& b) const {
		return state_hash == b.state_hash && reinference_required == b.reinference_required;
	}
};

struct networked_server_step_entropy {
	static constexpr bool force_read_field_by_field = true;

	// GEN INTROSPECTOR struct networked_server_step_entropy
	prestep_client_context context;
	server_step_entropy_meta meta;
	compact_server_step_entropy payload;
	// END GEN INTROSPECTOR

	bool operator==(const networked_server_step_entropy& b) const {
		return context == b.context && meta == b.meta && payload == b.payload;
	}
};
