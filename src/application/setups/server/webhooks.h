#pragma once
#include <cstddef>
#define RAPIDJSON_HAS_STDSTRING 1

#include "3rdparty/include_httplib.h"
#include "augs/string/parse_url.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "game/messages/hud_message.h"

std::string get_steam_join_link(const std::string& addr);
std::string get_browser_join_link(const std::string& addr);

namespace webhooks_common {
	inline std::string format_num(int num, std::size_t num_width = 2) {
		// todo: support larger numbers
		num = std::clamp(num, -9, 99);

		auto s = std::to_string(num);

		if (s.length() < num_width) {
			const auto n = num_width - s.length();
			auto preffix = std::string(n, ' ');

			s = preffix + s;
		}

		return s;
	}

	template<typename T>
	inline std::string make_stat_str(const T& entry) {
		const auto k = format_num(entry.kills);
		const auto a = format_num(entry.assists);
		const auto d = format_num(entry.deaths);

		return typesafe_sprintf("%x|%x|%x", k, a, d);
	}

	template<typename T>
	inline std::size_t get_longest_nickname_length(const T& members) {
		if (members.empty()) {
			return 0;
		}

		auto length_of = [](const auto& a, const auto& b) {
			return a.nickname.length() < b.nickname.length();
		};

		return maximum_of(members, length_of).nickname.length();
	}

	template<typename T, typename E>
	inline std::string make_team_members(const T& members, E&& escape_func, std::size_t nick_col_width = 0) {
		std::string output;

		for (const auto& m : members) {
			if (nick_col_width > 0) {
				// Discord webhook version with padding
				const auto padding = nick_col_width - m.nickname.length();
				output += escape_func(m.nickname) + std::string(padding, ' ') + make_stat_str(m) + "\n";
			} else {
				// Custom webhook version without padding
				output += escape_func(m.nickname) + " " + make_stat_str(m) + "\n";
			}
		}

		if (output.size() > 0) {
			output.pop_back();
		}

		return output;
	}

	inline std::string code_escaped_nick(const std::string& n) {
		std::string result;

		for (auto c : n) {
			if (c == '`') {
				result += "'";
				continue;
			}

			result += c;
		}

		return result;
	}
}

namespace discord_webhooks {
	inline std::string escaped_nick(const std::string& n) {
		std::string result;

		for (auto c : n) {
			if (c == '\n' || c == '\t' || c == '\r') {
				result += '?';
				continue;
			}

			switch(c) {
				case '\\':
				case '*':
				case '_':
				case '~':
				case '>':
				case '`':
					result += '\\';
					break;
				default:
					break;

			}

			result += c;
		}

		return result;
	}

}

inline std::string matrix_escaped_nick(const std::string& n, bool also_discord = true) {
	std::string result;

	for (auto c : n) {
		if (c == '\\' || c == '/') {
			result += '|';
			continue;
		}

		if (c == '<') {
			result += '{';
			continue;
		}

		if (c == '>') {
			result += '}';
			continue;
		}

		result += c;
	}

	if (also_discord) {
		return discord_webhooks::escaped_nick(result);
	}

	return result;
}

namespace telegram_webhooks {
	inline std::string escaped_nick(const std::string& n) {
		std::string result;
		/*
			Telegram sucks at sanitization so we have to whitelist
		*/

		const auto tg_nickname_whitelist = std::string("_'abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ ");

		for (auto c : n) {
			if (tg_nickname_whitelist.find(c) == std::string::npos) {
				result += '?';
			}
			else {
				result += c;
			}
		}

		return result;
	}

	inline httplib::UploadFormDataItems form_player_connected(
		const std::string& channel_id,
		const std::string& connected_player,
		const std::string& from_where
	) {
		const auto connected_notice = typesafe_sprintf("`%x` connected%x.", escaped_nick(connected_player), from_where);

		return {
			{ "chat_id", channel_id, "", "" },
			{ "parse_mode", "Markdown", "", "" },
			{ "text", connected_notice, "", "" }
		};
	}

	inline httplib::UploadFormDataItems form_error_report(
		const std::string& channel_id,
		const std::string& error
	) {
		return {
			{ "chat_id", channel_id, "", "" },
			{ "parse_mode", "Markdown", "", "" },
			{ "text", error, "", "" }
		};
	}

	inline httplib::UploadFormDataItems form_new_community_server(
		const std::string& channel_id,
		const std::string& new_server_name,
		const std::string& new_server_ip,
		const bool is_editor_playtesting_session,
		const bool is_web
	) {
#define SHOW_IPS_IN_WEBHOOKS 0

#if SHOW_IPS_IN_WEBHOOKS
		const std::string hosted_at = is_editor_playtesting_session ? " at " : " hosted at ";
		std::string full_description = "*" + escaped_nick(new_server_name) + "*" + hosted_at + "*" + new_server_ip + "*";
#else
		const std::string hosted = is_editor_playtesting_session ? "." : " hosted.";
		std::string full_description = "*" + escaped_nick(new_server_name) + "*" + hosted;
		(void)new_server_ip;
#endif

		if (is_web) {
			full_description += " (Web)";
		}

		return {
			{ "chat_id", channel_id, "", "" },
			{ "parse_mode", "Markdown", "", "" },
			{ "text", full_description, "", "" }
		};
	}
}

namespace custom_webhooks {
	inline httplib::Headers headers(const custom_webhook_data& data) {
		if (!data.header_authorization.empty()) {
			return {
				{"Authorization", data.header_authorization},
				{"Content-Type", "application/json"}
			};
		}
		else {
			return {
				{"Content-Type", "application/json"}
			};
		}
	}

	inline std::string message_player_connected(
		const std::string& server_name,
		const std::string& connected_player,
		std::vector<std::string> other_players,
		const std::string& current_map,
		const std::string& current_game_mode,
		const std::string& from_where,
		const std::string& external_addr,
		const bool compact
	) {
		erase_element(other_players, connected_player);

		const auto num_others = other_players.size();

		if (compact) {
			std::string result;
			if (num_others == 0) {
				// Alone
				result = typesafe_sprintf(
					"**%x** is playing.",
					matrix_escaped_nick(connected_player)
				);
			}
			else if (num_others == 1) {
				// One other player → show name
				result = typesafe_sprintf(
					"**%x** is playing with **%x**.",
					matrix_escaped_nick(connected_player),
					matrix_escaped_nick(other_players[0])
				);
			}
			else {
				// More than one other player → show count
				result = typesafe_sprintf(
					"**%x** is playing. **Total players: %x**.",
					matrix_escaped_nick(connected_player),
					num_others + 1
				);
			}

			if (!external_addr.empty()) {
				result += " [Join via Web](" + ::get_browser_join_link(external_addr) + ").";
			}

			return result;
		}

		const auto connected_notice = typesafe_sprintf("**%x** connected%x.", matrix_escaped_nick(connected_player), from_where);

		const auto now_playing_notice = [&]() {
			if (num_others == 0) {
				return typesafe_sprintf("");
			}

			if (num_others == 1) {
				return typesafe_sprintf("Now playing with **%x**.", matrix_escaped_nick(other_players[0]));
			}

			return typesafe_sprintf("Now playing with:");
		}();
			
		auto result = typesafe_sprintf("### `%x - %x (%x)`\n%x  \n%x  ", current_map, current_game_mode, server_name, connected_notice, now_playing_notice);

		if (num_others > 1) {
			std::string footer_content;

			for (const auto& p : other_players) {
				footer_content += "**" + matrix_escaped_nick(p) + "**  \n";
			}

			if (footer_content.size() > 0) {
				footer_content.pop_back();
			}

			result += "\n" + footer_content;
		}

		// Add join links if external address is available
		if (!external_addr.empty()) {
			result += "\n\n";
			result += "[Join via Web](" + ::get_browser_join_link(external_addr) + ") or _Steam_:  \n";
			result += "<pre><code>" + ::get_steam_join_link(external_addr) + "</pre></code>";
		}

		return result;
	}

	inline std::string message_duel_of_honor(
		const std::string& server_name,
		const std::string& first_player,
		const std::string& second_player,
		const std::string& current_map,
		const std::string& current_game_mode
	) {
		const auto duel_notice = typesafe_sprintf("**%x** and **%x** have agreed to a duel of honor.", matrix_escaped_nick(first_player), matrix_escaped_nick(second_player));
		
		auto result = typesafe_sprintf("### `%x - %x (%x)`\n%x", current_map, current_game_mode, server_name, duel_notice);

		return result;
	}

	inline std::string message_match_summary(
		const std::string& server_name,
		const std::string& mvp_nickname,
		const messages::match_summary_message& summary,
		const std::string& current_map,
		const std::string& current_game_mode,
		const std::string& external_addr
	) {
		const bool was_mvp_alone = summary.first_faction.size() == 1;
		const int num_against_mvp = summary.second_faction.size();
		const bool is_duel = was_mvp_alone && num_against_mvp == 1;

		const auto summary_notice = [&]() -> std::string {
			if (summary.was_ffa) {
				if (is_duel) {
					return "Duel has concluded.";
				}

				return "Free-for-all ended.";
			}

			const auto preffix = is_duel ? "Duel" : "Match";

			if (summary.is_tie()) {
				return typesafe_sprintf(
					"%x ended with a tie.",
					preffix
				);
			}

			return typesafe_sprintf(
				"%x ended with %x:%x.",
				preffix,
				summary.first_team_score,
				summary.second_team_score
			);
		}();

		const auto mvp_notice_suffix = [&]() -> std::string {
			if (summary.was_ffa) {
				if (is_duel) {
					return "has upheld his honor.";
				}
				return "was the strongest of them all.";
			}

			if (summary.is_tie()) {
				if (was_mvp_alone) {
					if (num_against_mvp > 1) {
						return "tied against overwhelming odds.";
					}

					if (num_against_mvp == 1) {
						/* If tied in a duel, no MVPs. */
						return "";
					}
				}
				else {
					return "was the strongest of them all.";
				}
			}

			if (was_mvp_alone) {
				if (num_against_mvp > 1) {
					return "won against overwhelming odds.";
				}

				if (num_against_mvp == 1) {
					return "has upheld his honor.";
				}
			}

			return "led his team to victory.";
		}();

		const auto mvp_notice = mvp_notice_suffix == "" ? "They stand on equal ground." : typesafe_sprintf(
			"**%x** %x",
			matrix_escaped_nick(mvp_nickname),
			mvp_notice_suffix
		);

		std::string total_description;
		total_description += mvp_notice;
		total_description += "\n\n";

		if (summary.was_ffa) {
			// For FFA matches, combine all players into one list
			std::vector<messages::match_summary_message::player_entry> all_players;
			all_players.insert(all_players.end(), summary.first_faction.begin(), summary.first_faction.end());
			all_players.insert(all_players.end(), summary.second_faction.begin(), summary.second_faction.end());

			const auto longest_nick_len = webhooks_common::get_longest_nickname_length(all_players);
			std::size_t nick_stat_padding = 8;
			std::size_t min_nick_col_width = 20;
			const auto nick_col_width = std::max(min_nick_col_width, longest_nick_len + nick_stat_padding);

			auto make_team_members = [&](const auto& members) {
				return webhooks_common::make_team_members(members, [](auto s) { return matrix_escaped_nick(s, false); }, nick_col_width);
			};

			total_description += "**Players:**\n";
			total_description += "<pre><code>";
			total_description += make_team_members(all_players);
			total_description += "</pre></code>";
		}
		else {
			// For team matches, show two separate teams
			const auto first_team_name = std::string(summary.is_tie() ? "First team:" : "Winning team:");
			const auto second_team_name = std::string(summary.is_tie() ? "Second team:" : "Losing team:");

			const auto longest_nick_len = std::max(
				webhooks_common::get_longest_nickname_length(summary.first_faction),
				webhooks_common::get_longest_nickname_length(summary.second_faction)
			);

			std::size_t nick_stat_padding = 8;
			std::size_t min_nick_col_width = 20;

			const auto nick_col_width = std::max(min_nick_col_width, longest_nick_len + nick_stat_padding);

			auto make_team_members = [&](const auto& members) {
				return webhooks_common::make_team_members(members, [](auto s) { return matrix_escaped_nick(s, false); }, nick_col_width);
			};

			total_description += "**" + first_team_name + "**\n";
			total_description += "<pre><code>";
			total_description += make_team_members(summary.first_faction);
			total_description += "</pre></code>\n\n";
			total_description += "**" + second_team_name + "**\n";
			total_description += "<pre><code>";
			total_description += make_team_members(summary.second_faction);
			total_description += "</pre></code>";
		}

		auto result = typesafe_sprintf("### `%x - %x (%x)`\n**%x**  \n%x", current_map, current_game_mode, server_name, summary_notice, total_description);

		if (!external_addr.empty()) {
			result += "\n\n";
			result += "[Join via Web](" + ::get_browser_join_link(external_addr) + ") or _Steam_:  \n";
			result += "<pre><code>" + ::get_steam_join_link(external_addr) + "</pre></code>";
		}

		return result;
	}

	inline std::string message_duel_interrupted(
		const std::string& server_name,
		const messages::duel_interrupted_message& info,
		const std::string& current_map,
		const std::string& current_game_mode
	) {
		using discord_webhooks::escaped_nick;

		const bool is_truce = info.is_truce();
		const bool was_winning = info.was_winning();

		const auto deserter = escaped_nick(info.deserter_nickname);
		const auto opponent = escaped_nick(info.opponent_nickname);

		const auto result_notice = [&]() -> std::string {
			if (is_truce) {
				return "Duel ended with a truce.";
			}

			return typesafe_sprintf("%x has surrendered.", deserter);
		}();
			
		const auto detail_notice = [&]() {
			if (is_truce) {
				return typesafe_sprintf(
					"**%x** %x %x:%x and no longer wants to fight **%x**.",
					deserter,
					was_winning ? "wins" : "ties",
					info.deserter_score,
					info.opponent_score,
					opponent
				);
			}

			return typesafe_sprintf(
				"**%x** was winning %x:%x...\nbut **%x** shamefully left the duel.",
				opponent,
				info.opponent_score,
				info.deserter_score,
				deserter
			);
		}();

		auto result = typesafe_sprintf("### `%x - %x (%x)`\n**%x**\n%x", current_map, current_game_mode, server_name, result_notice, detail_notice);

		return result;
	}

	inline std::string message_chat(
		const std::string& author,
		const std::string& message
	) {
		using webhooks_common::code_escaped_nick;
		return typesafe_sprintf("`%x: %x`", code_escaped_nick(author), code_escaped_nick(message));
	}
}

namespace discord_webhooks {
	inline std::string find_attachment_url(const std::string& response) {
		rapidjson::Document d;

		if (d.Parse(response).HasParseError()) {
			LOG("Couldn't parse JSON response.");
		}
		else {
			if (
				d.IsObject() 
				&& d.HasMember("attachments") 
				&& d["attachments"].IsArray()
				&& d["attachments"].Size() == 1
				&& d["attachments"][0].HasMember("url")
				&& d["attachments"][0]["url"].IsString()
			) {
				const auto& url = d["attachments"][0]["url"].GetString();

				LOG("Parsed JSON response successfully.");

				return url;
			}
			else {
				LOG("Couldn't access the url value.");
			}
		}

		return "";
	}

	inline httplib::UploadFormDataItems form_error_report(
		const std::string& username,
		const std::string& error_title,
		const std::string& error
	) {
		const auto payload = [&]()
		{
			using namespace rapidjson;

			const auto embed_color = 16763904;

			StringBuffer s;
			Writer<StringBuffer> writer(s);

			writer.StartObject();
			writer.Key("username");
			writer.String(username);
			writer.Key("embeds");
			writer.StartArray();
			{
				writer.StartObject();
				{
					writer.Key("title");
					writer.String(error_title);

					writer.Key("color");
					writer.Uint(embed_color);

					writer.Key("description");
					writer.String(error);
				}
				writer.EndObject();
			}
			writer.EndArray();
			writer.EndObject();

			return std::string(s.GetString());
		}();

		LOG("Generated payload: %x", payload);

		return {
			{ "payload_json", payload, "", "" }
		};
	}
	inline httplib::UploadFormDataItems form_new_community_server(
		const std::string& hook_username,
		const std::string& new_server_name,
		const std::string& new_server_ip,
		const std::string& arena_name,
		const std::string& game_mode,
		const int num_slots,
		const std::string& nat_type,
		const bool is_editor_playtesting_session,
		const bool is_web
	) {
		const auto payload = [&]()
		{
			using namespace rapidjson;
			using webhooks_common::code_escaped_nick;

			const auto embed_color = 16763904;

			std::string full_description;

			const bool show_ip = false;

			if (show_ip) {
				full_description += "IP:    " + code_escaped_nick(new_server_ip) + "\n";
			}

			full_description += "Map:   " + code_escaped_nick(arena_name) + "\n";
			full_description += "Mode:  " + code_escaped_nick(game_mode) + "\n";
			full_description += "Slots: " + std::to_string(num_slots) + "\n";
			full_description += "NAT:   " + code_escaped_nick(nat_type);
			full_description = "```" + full_description + "```";

			StringBuffer s;
			Writer<StringBuffer> writer(s);

			writer.StartObject();
			writer.Key("username");
			writer.String(hook_username);
			writer.Key("embeds");
			writer.StartArray();
			{
				writer.StartObject();
				{
					writer.Key("author");
					writer.StartObject();
					{
						writer.Key("name");

						auto name = std::string();

						if (is_editor_playtesting_session) {
							name = "New editor session!";
						}
						else {
							name = "New community server!";
						}

						if (is_web) {
							name += " (Web)";
						}

						writer.String(name.c_str());
					}
					writer.EndObject();

					writer.Key("title");
					writer.String(new_server_name);

					writer.Key("color");
					writer.Uint(embed_color);

					writer.Key("description");
					writer.String(full_description);
				}
				writer.EndObject();
			}
			writer.EndArray();
			writer.EndObject();

			return std::string(s.GetString());
		}();

		LOG("Generated payload: %x", payload);

		return {
			{ "payload_json", payload, "", "" }
		};
	}

	inline httplib::UploadFormDataItems form_duel_interrupted(
		const std::string& hook_username,
		const std::string& fled_pic_link,
		const std::string& truce_pic_link,
		const messages::duel_interrupted_message& info
	) {
		const auto payload = [&]()
		{
			using namespace rapidjson;

			const bool is_truce = info.is_truce();
			const bool was_winning = info.was_winning();

			const auto deserter = escaped_nick(info.deserter_nickname);
			const auto opponent = escaped_nick(info.opponent_nickname);

			const auto result_notice = [&]() -> std::string {
				if (is_truce) {
					return "Duel ended with a truce.";
				}

				return typesafe_sprintf("%x has surrendered.", deserter);
			}();
				
			const auto detail_notice = [&]() {
				if (is_truce) {
					return typesafe_sprintf(
						"**%x** %x %x:%x and no longer wants to fight **%x**.",
						deserter,
						was_winning ? "wins" : "ties",
						info.deserter_score,
						info.opponent_score,
						opponent
					);
				}

				return typesafe_sprintf(
					"**%x** was winning %x:%x...\nbut **%x** shamefully left the duel.",
					opponent,
					info.opponent_score,
					info.deserter_score,
					deserter
				);
			}();

			const auto& chosen_picture = is_truce ? truce_pic_link : fled_pic_link;

			StringBuffer s;
			Writer<StringBuffer> writer(s);

			writer.StartObject();
			writer.Key("username");
			writer.String(hook_username);
			writer.Key("embeds");
			writer.StartArray();
			{
				writer.StartObject();
				{
					writer.Key("title");
					writer.String(result_notice);

					writer.Key("image");
					writer.StartObject();
					{
						writer.Key("url");
						writer.String(chosen_picture);
					}
					writer.EndObject();

					writer.Key("description");
					writer.String(detail_notice);
				}
				writer.EndObject();
			}
			writer.EndArray();
			writer.EndObject();

			return std::string(s.GetString());
		}();

		LOG("Generated payload: %x", payload);

		return {
			{ "payload_json", payload, "", "" }
		};
	}

	inline httplib::UploadFormDataItems form_match_summary(
		const std::string& hook_username,
		const std::string& mvp_name,
		const std::string& mvp_avatar_link,
		const std::string& duel_victory_pic_link,
		const messages::match_summary_message& info
	) {
		const auto embed_color = 52479;

		const auto payload = [&]()
		{
			using namespace rapidjson;

			const bool was_mvp_alone = info.first_faction.size() == 1;
			const int num_against_mvp = info.second_faction.size();
			const bool is_duel = was_mvp_alone && num_against_mvp == 1;
			const bool is_duel_victory = is_duel && !info.is_tie();

			const auto summary_notice = [&]() -> std::string {
				if (info.was_ffa) {
					if (is_duel) {
						return "Duel has concluded.";
					}

					return "Free-for-all ended.";
				}

				const auto preffix = is_duel ? "Duel" : "Match";

				if (info.is_tie()) {
					return typesafe_sprintf(
						"%x ended with a tie.",
						preffix
					);
				}

				return typesafe_sprintf(
					"%x ended with %x:%x.",
					preffix,
					info.first_team_score,
					info.second_team_score
				);
			}();

			const auto mvp_notice_suffix = [&]() -> std::string {
				if (info.was_ffa) {
					if (is_duel) {
						return "has upheld his honor.";
					}

					return "was the strongest of them all.";
				}

				if (info.is_tie()) {
					if (was_mvp_alone) {
						if (num_against_mvp > 1) {
							return "tied against overwhelming odds.";
						}

						if (num_against_mvp == 1) {
							/* If tied in a duel, no MVPs. */
							return "";
						}
					}
					else {
						return "was the strongest of them all.";
					}
				}

				if (was_mvp_alone) {
					if (num_against_mvp > 1) {
						return "won against overwhelming odds.";
					}

					if (num_against_mvp == 1) {
						return "has upheld his honor.";
					}
				}

				return "led his team to victory.";
			}();

			const bool is_any_mvp = mvp_notice_suffix != "";
			const auto mvp_notice = mvp_notice_suffix == "" ? "They stand on equal ground." : typesafe_sprintf(
				"**%x** %x",
				escaped_nick(mvp_name),
				mvp_notice_suffix
			);

			std::string total_description;
			total_description += mvp_notice;
			total_description += "\n\n";

			if (info.was_ffa) {
				// For FFA matches, combine all players into one list
				std::vector<messages::match_summary_message::player_entry> all_players;
				all_players.insert(all_players.end(), info.first_faction.begin(), info.first_faction.end());
				all_players.insert(all_players.end(), info.second_faction.begin(), info.second_faction.end());

				const auto longest_nick_len = webhooks_common::get_longest_nickname_length(all_players);
				std::size_t nick_stat_padding = 8;
				std::size_t min_nick_col_width = 20;
				const auto nick_col_width = std::max(min_nick_col_width, longest_nick_len + nick_stat_padding);

				auto make_team_members = [&](const auto& members) {
					return webhooks_common::make_team_members(members, escaped_nick, nick_col_width);
				};

				total_description += "**Players:**";
				total_description += "```" + make_team_members(all_players) + "```";
			}
			else {
				// For team matches, show two separate teams
				const auto first_team_name = std::string (info.is_tie() ? "First team:" : "Winning team:");
				const auto second_team_name = std::string(info.is_tie() ? "Second team:" : "Losing team:");

				const auto longest_nick_len = std::max(
					webhooks_common::get_longest_nickname_length(info.first_faction),
					webhooks_common::get_longest_nickname_length(info.second_faction)
				);

				std::size_t nick_stat_padding = 8;
				std::size_t min_nick_col_width = 20;

				const auto nick_col_width = std::max(min_nick_col_width, longest_nick_len + nick_stat_padding);

				auto make_team_members = [&](const auto& members) {
					return webhooks_common::make_team_members(members, escaped_nick, nick_col_width);
				};

				total_description += "**" + first_team_name + "**";
				total_description += "```" + make_team_members(info.first_faction) + "```";
				total_description += "\n";
				total_description += "**" + second_team_name + "**";
				total_description += "```" + make_team_members(info.second_faction) + "```";
			}

			StringBuffer s;
			Writer<StringBuffer> writer(s);

			writer.StartObject();
			writer.Key("username");
			writer.String(hook_username);
			writer.Key("embeds");
			writer.StartArray();
			{
				writer.StartObject();
				{
					if (is_any_mvp && mvp_avatar_link.size() > 0) {
						writer.Key("thumbnail");
						writer.StartObject();
						{
							writer.Key("url");
							writer.String(mvp_avatar_link);
						}
						writer.EndObject();
					}

					if (is_duel_victory) {
						writer.Key("image");
						writer.StartObject();
						{
							writer.Key("url");
							writer.String(duel_victory_pic_link);
						}
						writer.EndObject();
					}

					writer.Key("title");
					writer.String(summary_notice);

					writer.Key("color");
					writer.Uint(embed_color);

					writer.Key("description");
					writer.String(total_description);
				}
				writer.EndObject();
			}
			writer.EndArray();
			writer.EndObject();

			return std::string(s.GetString());
		}();

		LOG("Generated payload: %x", payload);

		return {
			{ "payload_json", payload, "", "" }
		};
	}

	inline httplib::UploadFormDataItems form_duel_of_honor(
		const std::string& hook_username,
		const std::string& first_player,
		const std::string& second_player,
		const std::string& duel_picture_url
	) {
		const auto payload = [&]()
		{
			using namespace rapidjson;

			const auto duel_notice = typesafe_sprintf(
				"**%x** and **%x** have agreed to a duel of honor.",
				escaped_nick(first_player),
				escaped_nick(second_player)
			);

			StringBuffer s;
			Writer<StringBuffer> writer(s);

			writer.StartObject();
			writer.Key("username");
			writer.String(hook_username);
			writer.Key("embeds");
			writer.StartArray();
			{
				writer.StartObject();
				{
					writer.Key("image");
					writer.StartObject();
					{
						writer.Key("url");
						writer.String(duel_picture_url);
					}
					writer.EndObject();

					writer.Key("description");
					writer.String(duel_notice);
				}
				writer.EndObject();
			}
			writer.EndArray();
			writer.EndObject();

			return std::string(s.GetString());
		}();

		LOG("Generated payload: %x", payload);

		return {
			{ "payload_json", payload, "", "" }
		};
	}

	inline httplib::UploadFormDataItems form_player_connected(
		const std::vector<std::byte> avatar,
		const std::string& hook_username,
		const std::string& connected_player,
		std::vector<std::string> other_players,
		const std::string& current_map,
		const std::string& from_where
	) {
		erase_element(other_players, connected_player);

		const auto payload = [&]()
		{
			using namespace rapidjson;

			const auto connected_notice = typesafe_sprintf("%x connected%x.", escaped_nick(connected_player), from_where);
			const auto embed_color = 52224;
			const auto num_others = other_players.size();

			const auto now_playing_notice = [&]() {
				if (num_others == 0) {
					return typesafe_sprintf("Now playing ``%x``.", current_map);
				}

				if (num_others == 1) {
					return typesafe_sprintf("Now playing ``%x`` with **%x**.", current_map, escaped_nick(other_players[0]));
				}

				return typesafe_sprintf("Now playing ``%x`` with:", current_map);
			}();
				
			std::string footer_content;

			if (num_others > 1) {
				for (const auto& p : other_players) {
					footer_content += p + "\n";
				}

				if (footer_content.size() > 0) {
					footer_content.pop_back();
				}
			}

			StringBuffer s;
			Writer<StringBuffer> writer(s);

			writer.StartObject();
			writer.Key("username");
			writer.String(hook_username);
			writer.Key("embeds");
			writer.StartArray();
			{
				writer.StartObject();
				{
					writer.Key("color");
					writer.Uint(embed_color);

					writer.Key("title");
					writer.String(connected_notice);

					writer.Key("description");
					writer.String(now_playing_notice);

					if (!footer_content.empty()) {
						writer.Key("footer");
						writer.StartObject();
						{
							writer.Key("text");
							writer.String(footer_content);
						}
						writer.EndObject();
					}
				}
				writer.EndObject();
			}
			writer.EndArray();
			writer.EndObject();

			return std::string(s.GetString());
		}();

		LOG("Generated payload: %x", payload);

		if (avatar.empty()) {
			return {
				{ "payload_json", payload, "", "" }
			};
		}

		return {
			{ "payload_json", payload, "", "" },
			{ "file1", augs::bytes_to_string(avatar), "av.png", "application/octet-stream" }
		};
	}

}
