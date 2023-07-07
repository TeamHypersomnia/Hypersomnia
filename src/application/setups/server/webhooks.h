#pragma once
#define RAPIDJSON_HAS_STDSTRING 1

#include "3rdparty/include_httplib.h"
#include "augs/string/parse_url.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "game/messages/hud_message.h"

inline std::string bytes_to_string(const std::vector<std::byte>& bytes) {
	std::string result;
	result.resize(bytes.size());
	std::memcpy(result.data(), bytes.data(), bytes.size());

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

	inline httplib::MultipartFormDataItems form_player_connected(
		const std::string& channel_id,
		const std::string& connected_player
	) {
		const auto connected_notice = typesafe_sprintf("`%x` connected.", escaped_nick(connected_player));

		return {
			{ "chat_id", channel_id, "", "" },
			{ "parse_mode", "Markdown", "", "" },
			{ "text", connected_notice, "", "" }
		};
	}

	inline httplib::MultipartFormDataItems form_new_community_server(
		const std::string& channel_id,
		const std::string& new_server_name,
		const std::string& new_server_ip,
		const bool is_editor_playtesting_session
	) {
#define SHOW_IPS_IN_WEBHOOKS 0

#if SHOW_IPS_IN_WEBHOOKS
		const std::string hosted_at = is_editor_playtesting_session ? " at " : " hosted at ";
		const std::string full_description = "*" + escaped_nick(new_server_name) + "*" + hosted_at + "*" + new_server_ip + "*";
#else
		const std::string hosted = is_editor_playtesting_session ? "." : " hosted.";
		const std::string full_description = "*" + escaped_nick(new_server_name) + "*" + hosted;
		(void)new_server_ip;
#endif

		return {
			{ "chat_id", channel_id, "", "" },
			{ "parse_mode", "Markdown", "", "" },
			{ "text", full_description, "", "" }
		};
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

	inline httplib::MultipartFormDataItems form_new_community_server(
		const std::string& hook_username,
		const std::string& new_server_name,
		const std::string& new_server_ip,
		const std::string& arena_name,
		const std::string& game_mode,
		const int num_slots,
		const std::string& nat_type,
		const bool is_editor_playtesting_session
	) {
		const auto payload = [&]()
		{
			using namespace rapidjson;

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

						if (is_editor_playtesting_session) {
							writer.String("New editor session!");
						}
						else {
							writer.String("New community server!");
						}
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

	inline httplib::MultipartFormDataItems form_duel_interrupted(
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

	inline httplib::MultipartFormDataItems form_match_summary(
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

			const auto summary_notice = [&]() {
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

			const auto first_team_name = std::string(info.is_tie() ? "First team:" : "Winning team:");
			const auto second_team_name = std::string(info.is_tie() ? "Second team:" : "Losing team:");

			std::size_t num_width = 2;
			auto format_num = [&](auto num){
				// todo: support larger numbers
				num = std::clamp(num, -9, 99);

				auto s = std::to_string(num);

				if (s.length() < num_width) {
					const auto n = num_width - s.length();
					auto preffix = std::string(n, ' ');

					s = preffix + s;
				}

				return s;
			};

			auto make_stat_str = [&](const auto& entry) {
				const auto k = format_num(entry.kills);
				const auto a = format_num(entry.assists);
				const auto d = format_num(entry.deaths);

				return typesafe_sprintf("%x|%x|%x", k, a, d);
			};

			auto length_of = [](const auto& a, const auto& b) {
				return a.nickname.length() < b.nickname.length();
			};

			const auto longest_nick_len = std::max(
				info.first_faction.empty() ? std::size_t(0) : (maximum_of(info.first_faction,  length_of).nickname.length()),
				info.second_faction.empty() ? std::size_t(0) : (maximum_of(info.second_faction, length_of).nickname.length())
			);

			std::size_t nick_stat_padding = 8;
			std::size_t min_nick_col_width = 20;

			const auto nick_col_width = std::max(min_nick_col_width, longest_nick_len + nick_stat_padding);

			auto make_team_members = [&](const auto& members) {
				std::string output;

				for (const auto& m : members) {
					const auto padding = nick_col_width - m.nickname.length();
					output += code_escaped_nick(m.nickname) + std::string(padding, ' ') + make_stat_str(m) + "\n";
				}

				if (output.size() > 0) {
					output.pop_back();
				}

				return output;
			};

			std::string total_description;
			total_description += mvp_notice;
			total_description += "\n\n";
			total_description += "**" + first_team_name + "**";
			total_description += "```" + make_team_members(info.first_faction) + "```";
			total_description += "\n";
			total_description += "**" + second_team_name + "**";
			total_description += "```" + make_team_members(info.second_faction) + "```";

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

	inline httplib::MultipartFormDataItems form_duel_of_honor(
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

	inline httplib::MultipartFormDataItems form_player_connected(
		const std::vector<std::byte> avatar,
		const std::string& hook_username,
		const std::string& connected_player,
		std::vector<std::string> other_players,
		const std::string& current_map
	) {
		erase_element(other_players, connected_player);

		const auto payload = [&]()
		{
			using namespace rapidjson;

			const auto connected_notice = typesafe_sprintf("%x connected.", escaped_nick(connected_player));
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
			{ "file1", bytes_to_string(avatar), "av.png", "application/octet-stream" }
		};
	}

}
