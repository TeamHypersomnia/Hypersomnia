#pragma once
#include <vector>
#include "application/setups/server/chat_structs.h"
#include "augs/network/network_types.h"
#include "augs/math/vec2.h"
#include "augs/texture_atlas/atlas_entry.h"

namespace messages {
	struct mode_notification;
};

namespace augs {
	struct baked_font;
	struct drawer_with_default;
	class renderer;
}

struct client_chat_settings;
struct faction_view_settings;

struct server_broadcasted_chat;

struct chat_gui_entry {
	static chat_gui_entry from(
		const ::server_broadcasted_chat&,
		net_time_t timestamp,
		const std::string& author,
		const faction_type author_faction
	);

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
	std::vector<chat_gui_entry> history;

	template <class T>
	void add_entry(T&& entry) {
		history.emplace_back(std::forward<T>(entry));

		if (history.size() > 10000) {
			history.erase(history.begin(), history.begin() + 1000);
		}
	}

	bool add_entry_from_mode_notification(
		net_time_t,
		const messages::mode_notification&,
		const mode_player_id current_mode_id
	);

	bool perform_input_bar(const client_chat_settings&);

	void open_input_bar(chat_target_type);

	void draw_recent_messages(
		augs::drawer_with_default,
		const client_chat_settings&, 
		const faction_view_settings&,
		const augs::baked_font& gui_font,
		net_time_t current_time
	) const;
};
