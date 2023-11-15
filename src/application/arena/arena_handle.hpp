#pragma once

template <bool C, class ModeVariant, class RulesVariant>
class basic_arena_handle;

template <class A>
auto get_faction_of(const A& handle, const mode_player_id& id) {
	return handle.on_mode(
		[&](const auto& typed_mode) -> std::optional<faction_type> {
			const auto entry = typed_mode.find(id);

			if (entry == nullptr) {
				return std::nullopt;
			}

			return entry->get_faction();
		}
	);
}

template <class A>
bool is_spectator(const A& handle, const mode_player_id& id) {
	const auto faction = ::get_faction_of(handle, id);
	return faction == std::nullopt || faction == faction_type::SPECTATOR;
}

template <bool C, class M, class I>
bool found_in(const basic_arena_handle<C, M, I>& handle, const mode_player_id& id) {
	return handle.on_mode(
		[&](const auto& typed_mode) {
			return typed_mode.find(id) != nullptr;
		}
	);
}

template <bool C, class M, class I>
void get_arena_steam_rich_presence_pairs(
	steam_rich_presence_pairs& pairs,
	const std::string& mapname,
	const basic_arena_handle<C, M, I>& handle,
	const mode_player_id& local_id,
	const bool replaying
) {
	std::string gamemode;
	std::string score1;
	std::string score2;
	std::string level;
	bool spectating = false;
	std::string annotation;

	handle.on_mode_with_input(
		[&]<typename T>(const T& mode, const auto& input) {
			gamemode = mode.get_name(input);

			if (auto player_data = mode.find(local_id)) {
				if (player_data->get_faction() == faction_type::SPECTATOR) {
					spectating = true;
				}
				else {
					if constexpr(std::is_same_v<T, arena_mode>) {
						if (mode.get_state() == arena_mode_state::WARMUP) {
							annotation = "Warmup";
						}

						if (mode.levelling_enabled(input)) {
							level = std::to_string(player_data->stats.level);
						}
						else {
							auto participating = mode.calc_participating_factions(input);

							participating.for_each([&](const auto faction) {
								if (faction == player_data->get_faction()) {
									score1 = std::to_string(mode.get_faction_score(faction));
								}
								else {
									score2 = std::to_string(mode.get_faction_score(faction));
								}
							});
						}
					}
				}
			}
		}
	);

	pairs.push_back({ "mapname", mapname });
	pairs.push_back({ "gamemode", gamemode });

	if (spectating) {
		pairs.push_back({ "steam_display", "#Status_Spectating" });
	}
	else if (replaying) {
		pairs.push_back({ "steam_display", "#Status_Replaying" });
	}
	else {
		auto status = [&](auto fmt) {
			return typesafe_sprintf(fmt, "#Status_Playing");
		};

		if (!annotation.empty()) {
			pairs.push_back({ "steam_display", status("%xJustModeAnnotation") });
			pairs.push_back({ "annotation", annotation });
		}
		else if (!level.empty()) {
			pairs.push_back({ "steam_display", status("%xLevel") });
			pairs.push_back({ "level", level });
		}
		else if (!score1.empty() || !score2.empty()) {
			pairs.push_back({ "steam_display", status("%xTeam") });
			pairs.push_back({ "score1", score1 });
			pairs.push_back({ "score2", score2 });
		}
		else {
			pairs.push_back({ "steam_display", status("%xJustMode") });
		}
	}
}
