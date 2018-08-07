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

	auto& current_mode = cmd_in.get_player().current_mode;

	std::visit(
		[&](const auto& typed_mode) {
			auto& work = *cmd_in.folder.work;
			auto& cosm = work.world;

			auto in = commanding_property_editor_input {
				{ settings.property_editor, property_editor_data }, { cmd_in }
			};

			singular_edit_properties<
				change_current_mode_property_command
			>(
				in,
				typed_mode,
				" (Current mode)",
				special_widgets(
					flavour_widget { cosm }
				)
			);
		}, 
		current_mode
	);

	ImGui::Separator();

	//auto& all_vars = in.folder.mode_vars;
}
