#pragma once
#include "application/setups/server/chat_structs.h"
#include "augs/network/network_types.h"

struct chat_entry_type {
	net_time_t timestamp = 0.0;

	std::string author;
	faction_type author_faction = faction_type::DEFAULT;
	bool faction_specific = false;

	std::string message;
	rgba overridden_message_color = rgba::zero;

	std::string get_author_string() const;
	explicit operator std::string() const;
};

struct chat_gui_state {
	bool show = false;
	chat_target_type target = chat_target_type::GENERAL;

	std::string current_message;
	std::vector<chat_entry_type> history;
};
