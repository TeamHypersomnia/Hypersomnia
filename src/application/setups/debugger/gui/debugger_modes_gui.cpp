#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "game/cosmos/entity_handle.h"
#include "application/setups/debugger/debugger_folder.h"
#include "application/setups/debugger/gui/debugger_modes_gui.h"
#include "application/setups/debugger/debugger_command_input.h"
#include "application/setups/debugger/debugger_history.hpp"

#include "application/intercosm.h"
#include "application/setups/debugger/property_debugger/singular_edit_properties.h"
#include "application/setups/debugger/detail/field_address.h"

#include "application/setups/debugger/property_debugger/special_widgets.h"
#include "application/setups/debugger/property_debugger/widgets/flavour_widget.h"
#include "application/arena/arena_handle.h"

#include "application/setups/debugger/debugger_settings.h"

#include "augs/readwrite/byte_readwrite.h"
#include "augs/readwrite/memory_stream.h"

#include "application/setups/debugger/detail/maybe_different_colors.h"

#include "application/setups/debugger/property_debugger/widgets/pathed_asset_widget.h"
#include "application/setups/debugger/property_debugger/widgets/unpathed_asset_widget.h"
#include "application/setups/debugger/property_debugger/widgets/asset_sane_default_provider.h"

#define MACRO_MAKE_ONLY_TRIVIAL_FIELD_ADDRESS(a,b) make_field_address<only_trivial_field_type_id, decltype(a::b)>(augs_offsetof(a,b))

mode_entropy_general debugger_modes_gui::perform(const debugger_settings& settings, debugger_command_input cmd_in) {
	using namespace augs::imgui;

	auto window = make_scoped_window();

	if (!window) {
		return {};
	}

	mode_entropy_general output;

	acquire_keyboard_once();

	thread_local ImGuiTextFilter filter;
	filter.Draw();

	ImGui::Columns(2);
	next_columns(2);

	thread_local std::string nickname = "Test-player";

	auto& folder = cmd_in.folder;
	auto& player = folder.player;

	if (player.has_testing_started()) {
		player.get_arena_handle(folder).on_mode(
			[&](auto& typed_mode) {
				auto node = scoped_tree_node("Current mode state");

				next_columns(2);

				if (node) {
					using M = remove_cref<decltype(typed_mode)>;

					auto& work = cmd_in.folder.commanded->work;
					auto& cosm = work.world;

					const auto in = commanding_property_debugger_input {
						{ settings.property_debugger, property_debugger_data }, { cmd_in }
					};

					if constexpr(std::is_same_v<M, test_mode>) {
						(void)typed_mode;
					}
					else {
						if (ImGui::Button("Restart")) {
							mode_entropy_general cmd;
							cmd.special_command = match_command::RESTART_MATCH;

							output += cmd;
						}

						if (ImGui::Button("Add player")) {
							if (const auto new_id = typed_mode.find_first_free_player(); new_id.is_set()) {
								const auto new_name = typesafe_sprintf("Player%x", new_id.value);

								mode_entropy_general cmd;
								cmd.added_player = add_player_input {
									new_id, new_name, faction_type::DEFAULT
								};

								output += cmd;
							}
						}

						const auto players_node_label = "Players";
						auto players_node = scoped_tree_node(players_node_label);

						if (players_node) {
							for (const auto& p : typed_mode.get_players()) {
								const auto this_player_label = typesafe_sprintf("%x (id: %x)", p.second.get_nickname(), p.first.value);

								if (const auto this_player_node = scoped_tree_node(this_player_label.c_str())) {
									const auto player_handle = cosm[p.second.controlled_character_id];
									const auto character_name = player_handle.alive() ? player_handle.get_name() : "dead";
									text(typesafe_sprintf("Corresponding character name: %x", character_name));

									change_mode_player_property_command cmd;
									cmd.player_id = p.first;

									singular_edit_properties(
										in,
										p.second,
										this_player_label,
										cmd
									);
								}
							}
						}
					}

					singular_edit_properties(
						in,
						typed_mode,
						" (Current mode state)",
						change_current_mode_property_command(),
						special_widgets(
							flavour_widget { cosm }
						)
					);
				}
			}
		);
	}

	auto& rulesets = folder.commanded->rulesets.all;
	const auto& meta = folder.commanded->rulesets.meta;

	for_each_type_in_list<all_modes>(
		[&](auto m) {
			using M = decltype(m);

			ImGui::Separator();
			const auto& modes = rulesets.get_for<M>();
			const auto type_node_label = format_field_name(get_type_name<typename M::ruleset_type>()) + "s";

			auto type_node = scoped_tree_node(type_node_label.c_str());

			next_columns(2);

			if (type_node) {
				for (const auto& it : modes) {
					const auto& ruleset = it.second;
					const auto id = it.first;

					const auto typed_id = ruleset_id {
						mode_type_id::of<M>(),
						id
					};

					const bool is_server = meta.server_default == typed_id;
					const bool is_playtest = meta.playtest_default == typed_id;

					const auto node_label = ruleset.name + "###" + std::to_string(id);

					if (!filter.PassFilter(node_label.c_str())) {
						continue;
					}

					auto node = scoped_tree_node(node_label.c_str());

					if (is_server) {
						ImGui::SameLine();
						text_color("S", orange);
					}

					if (is_playtest) {
						ImGui::SameLine();
						text_color("P", yellow);
					}

					next_columns(2);

					if (node) {
						{
							auto f = is_playtest;

							auto scope = maybe_disabled_cols(settings.property_debugger, is_playtest);

							if (checkbox("Playtest default", f)) {
								if (!is_playtest) {
									change_rulesets_meta_property cmd;
									cmd.field = MACRO_MAKE_ONLY_TRIVIAL_FIELD_ADDRESS(rulesets_meta, playtest_default);
									augs::assign_bytes(cmd.value_after_change, typed_id);
									cmd.built_description = typesafe_sprintf("Selected \"%x\" as default for playtesting", ruleset.name);
									post_debugger_command(cmd_in, cmd);
								}
							}

							ImGui::NextColumn();
							ImGui::NextColumn();
						}

						{
							auto f = is_server;

							auto scope = maybe_disabled_cols(settings.property_debugger, is_server);

							if (checkbox("Server default", f)) {
								if (!is_server) {
									change_rulesets_meta_property cmd;
									cmd.field = MACRO_MAKE_ONLY_TRIVIAL_FIELD_ADDRESS(rulesets_meta, server_default);
									augs::assign_bytes(cmd.value_after_change, typed_id);
									cmd.built_description = typesafe_sprintf("Selected \"%x\" as default for servers", ruleset.name);
									post_debugger_command(cmd_in, cmd);
								}
							}

							ImGui::NextColumn();
							ImGui::NextColumn();
						}

						auto& work = folder.commanded->work;
						auto& cosm = work.world;

						auto in = commanding_property_debugger_input {
							{ settings.property_debugger, property_debugger_data }, { cmd_in }
						};

						change_ruleset_property_command cmd;
						cmd.type_id.set<M>();
						cmd.id = id;

						auto& defs = cmd_in.folder.commanded->work.viewables;

						const auto project_path = cmd_in.folder.current_path;

						const auto location = typesafe_sprintf(" (%x)", ruleset.name);

						singular_edit_properties(
							in,
							ruleset,
							location.c_str(),
							cmd,
							special_widgets(
								pathed_asset_widget { defs, project_path, cmd_in },
								unpathed_asset_widget { defs, cosm.get_logical_assets() },
								flavour_widget { cosm }
							),

							asset_sane_default_provider { defs }
						);
					}
				}
			}
		}
	);

	return output;
}
