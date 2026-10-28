#pragma once
#include <vector>
#include "augs/filesystem/path.h"
#include "augs/readwrite/json_readwrite.h"
#include "augs/string/hex.h"
#include "augs/network/netcode_utils.h"
#include "application/main/favorite_servers.h"
#include "application/gui/server_list_entry.h"

/*
	The favorites' last heartbeats, by their key and address - next to the server list's cache. Read once, written a few
	seconds after one changed. Only what a favorite tournament needs before it answers again: its name, its stage,
	its ports.
*/

class favorite_heartbeats {
	augs::path_type path;
	std::vector<favorite_heartbeat_entry> entries;
	bool loaded = false;
	bool dirty = false;
	double when_changed = 0.0;

	static favorite_heartbeat_entry make_entry(const server_list_entry& s) {
		const auto& d = s.heartbeat;

		auto e = favorite_heartbeat_entry();
		e.key = augs::bytes_to_hex(s.meta.server_public_key);
		e.address = ::ToString(s.address);
		e.arena = std::string(d.current_arena);
		e.game_mode = std::string(d.game_mode);
		e.ranked_state = d.ranked_state;
		e.tournament_name = std::string(d.tournament_name);
		e.tournament_stage = d.tournament_stage;
		e.finished = d.finished;
		e.tournament_page_port = d.tournament_page_port;
		e.instance_first_port = d.instance_first_port;
		e.instance_num_ports = d.instance_num_ports;
		return e;
	}

public:
	void load_once(const augs::path_type& from) {
		if (loaded) {
			return;
		}

		loaded = true;
		path = from;

		try {
			entries = augs::from_json_file<std::vector<favorite_heartbeat_entry>>(path);
		}
		catch (...) {
			entries.clear();
		}
	}

	/*
		A favorite that answered: its heartbeat remembered.
	*/

	void remember(const server_list_entry& s, const double now) {
		const auto e = make_entry(s);

		for (auto& existing : entries) {
			if (existing.key == e.key && existing.address == e.address) {
				if (!(existing == e)) {
					existing = e;
					dirty = true;
					when_changed = now;
				}

				return;
			}
		}

		entries.push_back(e);
		dirty = true;
		when_changed = now;
	}

	/*
		A favorite not answered yet this time: what it last told - its details still unknown (gray).
	*/

	void apply_to(server_list_entry& s) const {
		const auto key = augs::bytes_to_hex(s.meta.server_public_key);
		const auto address = ::ToString(s.address);

		for (const auto& e : entries) {
			if (e.key == key && e.address == address) {
				auto& d = s.heartbeat;

				d.current_arena = e.arena;
				d.game_mode = e.game_mode;
				d.ranked_state = e.ranked_state;
				d.tournament_name = e.tournament_name;
				d.tournament_stage = static_cast<int8_t>(e.tournament_stage);
				d.finished = e.finished;
				d.tournament_page_port = e.tournament_page_port;
				d.instance_first_port = e.instance_first_port;
				d.instance_num_ports = e.instance_num_ports;
				return;
			}
		}
	}

	void save_if_changed(const double now) {
		constexpr auto save_after_secs = 3.0;

		if (!dirty || now - when_changed < save_after_secs) {
			return;
		}

		dirty = false;

		try {
			augs::create_directories_for(path);
			augs::save_as_json_atomic(entries, path);
		}
		catch (const std::exception& err) {
			LOG("Could not save the favorites' heartbeats to %x: %x", path, err.what());
		}
	}
};
