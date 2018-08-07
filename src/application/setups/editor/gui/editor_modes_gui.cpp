#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "application/setups/editor/editor_folder.h"
#include "application/setups/editor/gui/editor_modes_gui.h"
#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/editor_history.hpp"

#include "application/intercosm.h"
#include "application/setups/editor/property_editor/singular_edit_properties.h"
#include "application/setups/editor/detail/field_address.h"

#include "application/setups/editor/property_editor/special_widgets.h"
#include "application/setups/editor/property_editor/widgets/flavour_widget.h"

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

	ImGui::Columns(2); // 4-ways, with border
	next_column_text_disabled("Details");

	{
		auto node = scoped_tree_node("Current mode");

		next_columns(2);

		if (node) {
			auto& current_mode = cmd_in.get_player().current_mode;

			std::visit(
				[&](const auto& typed_mode) {
					auto& work = *cmd_in.folder.work;
					auto& cosm = work.world;

					auto in = commanding_property_editor_input {
						{ settings.property_editor, property_editor_data }, { cmd_in }
					};

					singular_edit_properties(
						in,
						typed_mode,
						" (Current mode)",
						change_current_mode_property_command(),
						special_widgets(
							flavour_widget { cosm }
						)
					);
				}, 
				current_mode
			);
		}
	}

	ImGui::Separator();

	auto& all_vars = cmd_in.folder.mode_vars;

	all_vars.for_each_container(
		[&](const auto& modes) {
			ImGui::Separator();
			using vars_type = typename remove_cref<decltype(modes)>::mapped_type;

			const auto type_node_label = format_field_name(get_type_name<vars_type>());
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
						auto& work = *cmd_in.folder.work;
						auto& cosm = work.world;

						auto in = commanding_property_editor_input {
							{ settings.property_editor, property_editor_data }, { cmd_in }
						};

						change_mode_vars_property_command cmd;
						cmd.vars_type_id.set<vars_type>();
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
