#pragma once
#include <vector>
#include "application/setups/server/chat_structs.h"
#include "augs/network/network_types.h"
#include "augs/math/vec2.h"

namespace augs {
	struct baked_font;
	struct drawer;
}

struct client_chat_settings;
struct faction_view_settings;

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

	template <class T>
	void add_entry(T&& entry) {
		history.emplace_back(std::forward<T>(entry));

		if (history.size() > 10000) {
			history.erase(history.begin(), history.begin() + 1000);
		}
	}

	bool perform_input_bar(const client_chat_settings&);

	void open_input_bar(chat_target_type);

	void draw_recent_messages(
		augs::drawer,
		const client_chat_settings&, 
		const faction_view_settings&,
		const augs::baked_font& gui_font,
		net_time_t current_time
	) const;
};
