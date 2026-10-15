#pragma once
#include "augs/misc/date_time.h"
#include "augs/misc/secure_hash.h"
#include "augs/string/base64.h"

namespace ranked_webhooks {
	/*
		get_avatar_of gives a player's avatar as PNG bytes (empty for none) - the site keeps the last one of every account.
	*/

	template <class F, class A>
	std::string json_report_match(
		const std::string& server_name,
		const std::string& arena,
		const std::string& game_mode,
		const messages::match_summary_message& info,
		const bool happy_hours,
		F get_clan_of,
		A get_avatar_of
	) {
		using namespace rapidjson;

		StringBuffer s;
		Writer<StringBuffer> writer(s);

		writer.StartObject();

		/* Hex string of the random 64-bit match id, for deduplicating reports (e.g. after a crash recovery). */
		writer.Key("match_id");
		writer.String(augs::bytes_to_hex(info.ranked_match_id).c_str());

		/* When the report was made: the site accepts it for a few days, and the signature covers it. */
		writer.Key("reported_at");
		writer.Uint64(static_cast<uint64_t>(augs::date_time::secs_since_epoch()));

		writer.Key("match_start_date");
		writer.String(info.match_start_timestamp.c_str());

		/* Happy hours by the server's own time zone when the match started - the site doubles the gains then. */
		writer.Key("event_match_multiplier");
		writer.Int(happy_hours ? 2 : 1);

		writer.Key("server_name");
		writer.String(server_name.c_str());

		writer.Key("arena");
		writer.String(arena.c_str());

		writer.Key("game_mode");
		writer.String(game_mode.c_str());

		writer.Key("win_score");
		writer.Int(info.first_team_score);

		writer.Key("lose_score");
		writer.Int(info.second_team_score);

		writer.Key("losers_abandoned");
		writer.Bool(info.losers_abandoned);

		writer.Key("player_infos");

		{
			writer.StartObject();

			auto write_faction = [&](const auto& faction) {
				for (const auto& f : faction) {
					writer.Key(f.account_id);

					{
						writer.StartObject();
						writer.Key("nickname");
						writer.String(f.nickname.c_str());
						writer.Key("clan");
						writer.String(get_clan_of(f.id));

						if (f.abandoned_at_score != -1) {
							writer.Key("abandoned_at_score");
							writer.Int(f.abandoned_at_score);
						}

						if (const auto avatar = get_avatar_of(f.id); !avatar.empty()) {
							writer.Key("avatar");
							writer.String(augs::to_base64(avatar).c_str());
						}

						writer.EndObject();
					}
				}
			};

			write_faction(info.first_faction);
			write_faction(info.second_faction);

			writer.EndObject();
		}

		auto write_faction_ids = [&](const auto& faction) {
			writer.StartArray();

			for (const auto& f : faction) {
				writer.String(f.account_id.c_str());
			}

			writer.EndArray();
		};

		writer.Key("win_players");
		write_faction_ids(info.first_faction);

		writer.Key("lose_players");
		write_faction_ids(info.second_faction);

		writer.EndObject();

		return std::string(s.GetString());
	}
}

