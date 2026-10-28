#pragma once
#include <vector>
#include <optional>
#include <algorithm>
#include "application/gui/server_list_entry.h"

/*
	The server browser draws a tournament's matches under one row of its own - collapsed by default, expanded by a click.
	Its matches: the same key, at the same IP, of the same first port (server_heartbeat::instance_first_port) -
	nothing is kept of them between the frames but which tournaments are expanded.
*/

struct tournament_group_id {
	augs::ed25519_public_key_type key;

	/*
		Its first instance: the IP, the first port - where a favorite tournament is.
	*/

	netcode_address_t first_instance;

	bool operator==(const tournament_group_id& b) const {
		return key == b.key && first_instance == b.first_instance;
	}
};

inline std::optional<tournament_group_id> tournament_group_of(const server_list_entry& s) {
	const auto& d = s.heartbeat;

	if (!d.is_tournament_server() || d.instance_first_port == 0) {
		return std::nullopt;
	}

	const auto key = s.get_public_key();

	if (!key.has_value()) {
		return std::nullopt;
	}

	auto first = s.address;
	first.port = d.instance_first_port;

	return tournament_group_id { *key, first };
}

struct browser_row {
	std::optional<tournament_group_id> group;

	/*
		One server - or a tournament's matches, in the order the list had them.
	*/

	std::vector<server_list_entry*> entries;

	bool is_tournament() const {
		return group.has_value();
	}

	/*
		The whole tournament over: only its final is left, finished - not expanded.
	*/

	bool is_finished_tournament() const {
		return is_tournament() && entries.size() == 1 && entries[0]->heartbeat.is_finished_tournament();
	}

	/*
		Its first instance, if here - what its ping and its star are.
	*/

	const server_list_entry* first_instance() const {
		for (const auto e : entries) {
			if (e->address.port == group->first_instance.port) {
				return e;
			}
		}

		return nullptr;
	}
};

/*
	A row per server, the tournaments' matches gathered into the row of the first of them on the (sorted) list.
*/

inline std::vector<browser_row> make_browser_rows(const std::vector<server_list_entry*>& sorted) {
	auto rows = std::vector<browser_row>();

	for (const auto s : sorted) {
		if (const auto group = ::tournament_group_of(*s)) {
			const auto existing = std::find_if(rows.begin(), rows.end(), [&](const browser_row& r) {
				return r.group == group;
			});

			if (existing != rows.end()) {
				existing->entries.push_back(s);
				continue;
			}

			rows.push_back({ group, { s } });
			continue;
		}

		rows.push_back({ std::nullopt, { s } });
	}

	return rows;
}
