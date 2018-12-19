#pragma once
#include "game/modes/mode_entropy.h"

using server_step_entropy = mode_entropy;

struct prestep_client_context {
	uint8_t num_entropies_accepted = 1;
};

struct client_entropy_entry {
	mode_player_id player_id;
	total_mode_player_entropy total;
};

struct networked_server_step_entropy {
	std::vector<client_entropy_entry> players;
	mode_entropy_general general;

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

			if (!t.mode.empty()) {
				out.players[p.player_id] = t.mode;
			}

			if (!t.cosmic.empty()) {
				const auto id = mode_id_to_entity_id(p.player_id);

				if (id.is_set()) {
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

