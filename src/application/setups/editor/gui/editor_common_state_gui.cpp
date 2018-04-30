#include "application/setups/editor/gui/editor_common_state_gui.h"
#include "application/setups/editor/editor_command_input.h"

#if BUILD_PROPERTY_EDITOR

#include "augs/misc/simple_pair.h"
#include "augs/templates/for_each_std_get.h"

#include "augs/readwrite/memory_stream.h"

#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "application/intercosm.h"
#include "application/setups/editor/editor_folder.h"

#include "application/setups/editor/property_editor/general_edit_properties.h"
#include "application/setups/editor/detail/field_address.h"
#include "application/setups/editor/property_editor/assets/pathed_asset_widget.h"
#include "application/setups/editor/property_editor/commanding_property_editor_input.h"

#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/byte_readwrite.h"

template <class T>
struct should_skip_in_common : std::bool_constant<
	is_one_of_v<T, all_logical_assets, all_entity_flavours>
> {};

static void edit_common(
	const commanding_property_editor_input& in,
	const cosmos_common_significant& signi
) {
	using namespace augs::imgui;

	const auto property_location = [&]() {
		return typesafe_sprintf(" (Common state)");
	}();

	auto& cmd_in = in.command_in;
	/* Linker error fix */
	auto& history = cmd_in.folder.history;

	auto post_new_change = [&](
		const auto& description,
		const field_address field,
		const auto& new_content
	) {
		change_common_state_command cmd;
		cmd.field = field;

		cmd.value_after_change = augs::to_bytes(new_content);
		cmd.built_description = description + property_location;

		history.execute_new(cmd, cmd_in);
	};

	auto rewrite_last_change = [&](
		const auto& description,
		const auto& new_content
	) {
		auto& last = history.last_command();

		if (auto* const cmd = std::get_if<change_common_state_command>(std::addressof(last))) {
			cmd->built_description = description + property_location;
			cmd->rewrite_change(augs::to_bytes(new_content), cmd_in);
		}
		else {
			LOG("WARNING! There was some problem with tracking activity of editor controls.");
		}
	};

	auto& defs = cmd_in.folder.work->viewables;
	const auto project_path = cmd_in.folder.current_path;

	general_edit_properties<should_skip_in_common>(
		in.prop_in, 
		signi,
		post_new_change,
		rewrite_last_change,
		true_returner(),
		pathed_asset_widget { defs, project_path, cmd_in }
	);
}

void editor_common_state_gui::perform(const editor_settings& settings, const editor_command_input in) {
	using namespace augs::imgui;

	auto window = make_scoped_window();
	
	if (!window) {
		return;
	}

	auto& work = *in.folder.work;
	auto& cosm = work.world;

	ImGui::Columns(2); // 4-ways, with border
	next_column_text_disabled("Details");
	ImGui::Separator();

	edit_common(
		{ { settings.property_editor, property_editor_data }, in },
		cosm.get_common_significant()
	);
}
#else
void editor_common_state_gui::perform(const editor_settings& settings, const editor_command_input in) {

}
#endif
