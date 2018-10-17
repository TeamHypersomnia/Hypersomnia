#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "game/cosmos/entity_handle.h"
#include "application/setups/editor/editor_folder.h"
#include "application/setups/editor/gui/editor_modes_gui.h"
#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/editor_history.hpp"

#include "application/intercosm.h"
#include "application/setups/editor/property_editor/singular_edit_properties.h"
#include "application/setups/editor/detail/field_address.h"

#include "application/setups/editor/property_editor/special_widgets.h"
#include "application/setups/editor/property_editor/widgets/flavour_widget.h"

#include "application/setups/editor/editor_player.hpp"
#include "application/setups/editor/editor_settings.h"

#include "augs/readwrite/byte_readwrite.h"
#include "augs/readwrite/memory_stream.h"

void editor_modes_gui::perform(const editor_settings& settings, editor_command_input cmd_in) {
	using namespace augs::imgui;

	auto window = make_scoped_window();

	if (!window) {
		return;
	}

	acquire_keyboard_once();

	thread_local ImGuiTextFilter filter;
	filter.Draw();

	ImGui::Columns(2);
	next_column_text_disabled("Details");

	thread_local std::string nickname = "Test-player";

	/* TODO: commandize it properly!!! */

	auto& folder = cmd_in.folder;
	auto& player = folder.player;

	player.on_mode_with_input(
		folder.commanded->mode_vars.vars,
		folder.commanded->work.world,
		[&](auto& typed_mode, const auto& mode_input) {
			auto node = scoped_tree_node("Current mode");

			next_columns(2);

			if (node) {
				using M = remove_cref<decltype(typed_mode)>;

				auto& work = cmd_in.folder.commanded->work;
				auto& cosm = work.world;

				const auto in = commanding_property_editor_input {
					{ settings.property_editor, property_editor_data }, { cmd_in }
				};

				if constexpr(std::is_same_v<M, test_scene_mode>) {
					(void)mode_input;
				}
				else {
					if (ImGui::Button("Add player")) {
						typed_mode.auto_assign_faction(mode_input, typed_mode.add_player(mode_input, nickname));
					}

					ImGui::SameLine();

					if (ImGui::Button("Add spectator")) {
						typed_mode.add_player(mode_input, nickname);
					}

					ImGui::SameLine();

					input_text<256>("Nickname", nickname);

					if (ImGui::Button("Restart")) {
						typed_mode.request_restart();
					}

					const auto players_node_label = "Players";
					auto players_node = scoped_tree_node(players_node_label);

					if (players_node) {
						for (const auto& p : typed_mode.players) {
							const auto this_player_label = typesafe_sprintf("%x (id: %x)", p.second.chosen_name, p.first.value);

							if (const auto this_player_node = scoped_tree_node(this_player_label.c_str())) {
								const auto player_handle = cosm[p.second.guid];
								const auto character_name = player_handle.alive() ? player_handle.get_name() : "dead";
								text(typesafe_sprintf("Corresponding character name: %x", character_name));

								singular_edit_properties(
									in,
									p.second,
									this_player_label,
									change_current_mode_property_command()
								);
							}
						}
					}
				}

				singular_edit_properties(
					in,
					typed_mode,
					" (Current mode)",
					change_current_mode_property_command(),
					special_widgets(
						flavour_widget { cosm }
					)
				);
			}
		}
	);

	ImGui::Separator();

	auto& all_vars = folder.commanded->mode_vars;

	for_each_type_in_list<all_modes>(
		[&](auto m) {
			using M = decltype(m);

			ImGui::Separator();
			const auto& modes = all_vars.vars.get_for<M>();
			const auto type_node_label = format_field_name(get_type_name<typename M::vars_type>());

			auto type_node = scoped_tree_node(type_node_label.c_str());

			next_columns(2);

			if (type_node) {
				for (const auto& it : modes) {
					const auto& typed_mode = it.second;
					const auto vars_id = it.first;
					const auto node_label = typed_mode.name + "###" + std::to_string(vars_id);
					auto node = scoped_tree_node(node_label.c_str());

					next_columns(2);

					if (node) {
						auto& work = folder.commanded->work;
						auto& cosm = work.world;

						auto in = commanding_property_editor_input {
							{ settings.property_editor, property_editor_data }, { cmd_in }
						};

						change_mode_vars_property_command cmd;
						cmd.vars_type_id.set<M>();
						cmd.vars_id = vars_id;

						singular_edit_properties(
							in,
							typed_mode,
							" (Current mode)",
							cmd,
							special_widgets(
								flavour_widget { cosm }
							)
						);
					}
				}
			}
		}
	);
}
