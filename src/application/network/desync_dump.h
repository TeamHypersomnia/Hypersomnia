#pragma once
#include <cstdint>
#include <exception>
#include <iomanip>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <variant>

#include "augs/log.h"
#include "augs/log_path_getters.h"
#include "augs/filesystem/file.h"
#include "augs/misc/date_time.h"
#include "augs/string/get_type_name.h"
#include "augs/string/typesafe_sprintf.h"
#include "augs/templates/enum_introspect.h"
#include "game/cosmos/cosmos.h"
#include "game/modes/arena_mode.h"

/*
	Written by the client only when it desyncs: logs/desync.<time>.step<step>.txt.

	The state hash (cosmos::calculate_solvable_signi_hash) covers the entity count and, of every character,
	the body's transform and the meters - so the file lists exactly those. First the client's own, after the rest
	of the steps of the frame that found the desync; then, once the server's resync snapshot arrives, the client's
	right before it next to the server's from it. The character whose lines differ is the one that diverged.
	Bots also list their paths, which are mode state outside the hash: another path to the same target points at the pathfinding.

	Compare the steps in the headers of the last two sections: when the client's is lower, its queued steps were dropped
	for the snapshot and characters in motion differ slightly everywhere - the diverged one differs much more, or in its path.
*/

inline std::string desync_dump_exact(const real32 value) {
	auto out = std::ostringstream();
	out << std::setprecision(std::numeric_limits<real32>::max_digits10) << value;
	return out.str();
}

inline std::string desync_dump_exact(const vec2 value) {
	return "(" + ::desync_dump_exact(value.x) + ", " + ::desync_dump_exact(value.y) + ")";
}

inline std::string desync_dump_exact(const value_meter& meter) {
	return typesafe_sprintf(
		"%x/%x (+%x per %x ms)",
		::desync_dump_exact(meter.value),
		::desync_dump_exact(meter.maximum),
		::desync_dump_exact(meter.regeneration_unit),
		::desync_dump_exact(meter.regeneration_interval_ms)
	);
}

template <class ArenaHandle>
std::string make_desync_dump_section(const std::string& title, const ArenaHandle& arena) {
	const auto& cosm = arena.get_cosmos();

	auto out = typesafe_sprintf(
		"== %x (step %x, entities %x)\n\nCharacters (body transform p/q in meters as hashed, velocity, meters):\n",
		title,
		cosm.get_total_steps_passed(),
		cosm.get_entities_count()
	);

	arena.on_mode([&](const auto& typed_mode) {
		using M = remove_cref<decltype(typed_mode)>;

		auto player_of = std::map<entity_id, const arena_mode_player*>();

		if constexpr(std::is_same_v<M, arena_mode>) {
			for (const auto& p : typed_mode.get_players()) {
				player_of[p.second.controlled_character_id] = &p.second;
			}
		}

		cosm.template for_each_having<components::sentience>([&](const auto& typed_character) {
			const auto id = typed_character.get_id();
			const auto& body = typed_character.template get<components::rigid_body>().get_raw_component();
			const auto& xf = body.physics_transforms.m_xf;
			const auto& sentience = typed_character.template get<components::sentience>();

			const auto who = [&]() -> std::string {
				if (const auto found = player_of.find(id); found != player_of.end()) {
					const auto& player = *found->second;

					return typesafe_sprintf(
						"%x (%x, %x)",
						player.get_nickname(),
						player.is_bot ? "bot" : "human",
						augs::enum_to_string(player.get_faction())
					);
				}

				return "no player";
			}();

			out += typesafe_sprintf(
				"  %x %x: p %x q %x vel %x hp %x pe %x cp %x\n",
				id,
				who,
				::desync_dump_exact(vec2(xf.p.x, xf.p.y)),
				::desync_dump_exact(vec2(xf.q.s, xf.q.c)),
				::desync_dump_exact(body.velocity),
				::desync_dump_exact(sentience.template get<health_meter_instance>()),
				::desync_dump_exact(sentience.template get<personal_electricity_meter_instance>()),
				::desync_dump_exact(sentience.template get<consciousness_meter_instance>())
			);
		});

		if constexpr(std::is_same_v<M, arena_mode>) {
			out += "\nBots (behavior, navigation target, path cells):\n";

			for (const auto& p : typed_mode.get_players()) {
				const auto& player = p.second;

				if (!player.is_bot) {
					continue;
				}

				const auto& ai = player.ai_state;

				const auto behavior = std::visit(
					[](const auto& typed_behavior) {
						return get_type_name_strip_namespace<remove_cref<decltype(typed_behavior)>>();
					},
					ai.last_behavior
				);

				out += typesafe_sprintf("  %x: %x", player.get_nickname(), behavior);

				if (ai.navigation.has_value()) {
					const auto& navigation = *ai.navigation;
					const auto& path = navigation.main.path;

					out += typesafe_sprintf(
						", target %x, island %x, at node %x of %x:",
						::desync_dump_exact(navigation.target_transform.pos),
						static_cast<int>(path.island_index),
						navigation.main.node_index,
						path.nodes.size()
					);

					for (const auto& node : path.nodes) {
						out += typesafe_sprintf(" %x", node.cell_xy);
					}

					if (navigation.rerouting.has_value()) {
						out += typesafe_sprintf(", rerouting through %x nodes", navigation.rerouting->path.nodes.size());
					}
				}
				else {
					out += ", no path";
				}

				out += "\n";
			}
		}
	});

	return out + "\n";
}

struct desync_dump {
	augs::path_type path;
	std::string text;

	template <class ArenaHandle>
	void on_desync(
		const ArenaHandle& referential,
		const uint32_t step,
		const uint32_t expected_hash,
		const uint32_t actual_hash
	) {
		path = get_log_file_path(typesafe_sprintf("desync.%x.step%x.txt", augs::date_time().get_readable_for_file_long(), step));

		text = typesafe_sprintf("Desync detected at step %x. Server's hash: %x, client's: %x.\n\n", step, expected_hash, actual_hash);
		text += ::make_desync_dump_section("Client after the rest of the steps received in that frame", referential);

		save();
	}

	template <class ArenaHandle>
	void before_resync_snapshot(const ArenaHandle& referential) {
		if (path.empty()) {
			return;
		}

		text += ::make_desync_dump_section("Client right before the resync snapshot", referential);
	}

	template <class ArenaHandle>
	void after_resync_snapshot(const ArenaHandle& referential) {
		if (path.empty()) {
			return;
		}

		text += ::make_desync_dump_section("Server (the resync snapshot)", referential);

		save();

		path.clear();
		text.clear();
	}

	void save() const {
		try {
			augs::save_as_text(path, text);
			LOG("Desync details written to %x", path);
		}
		catch (const std::exception& e) {
			LOG("Could not write the desync details to %x: %x", path, e.what());
		}
	}
};
