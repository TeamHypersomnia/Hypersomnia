#include "application/setups/debugger/gui/debugger_common_state_gui.h"
#include "application/setups/debugger/debugger_command_input.h"

#if BUILD_PROPERTY_DEBUGGER

#include "augs/misc/simple_pair.h"
#include "augs/templates/for_each_std_get.h"
#include "application/setups/debugger/debugger_history.hpp"

#include "augs/readwrite/memory_stream.h"

#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "application/intercosm.h"
#include "application/setups/debugger/debugger_folder.h"

#include "application/setups/debugger/property_debugger/singular_edit_properties.h"
#include "application/setups/debugger/detail/field_address.h"
#include "application/setups/debugger/property_debugger/widgets/pathed_asset_widget.h"
#include "application/setups/debugger/property_debugger/widgets/flavour_widget.h"
#include "application/setups/debugger/property_debugger/widgets/unpathed_asset_widget.h"
#include "application/setups/debugger/property_debugger/special_widgets.h"
#include "application/setups/debugger/property_debugger/commanding_property_debugger_input.h"
#include "application/setups/debugger/debugger_settings.h"

#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/byte_readwrite.h"

struct common_state_debugger_behaviour {
	template <class T>
	static constexpr bool should_skip = is_one_of_v<T, all_logical_assets, all_entity_flavours>;
};

static void edit_common(
	const commanding_property_debugger_input& in,
	const cosmos_common_significant& signi
) {
	auto& cmd_in = in.command_in;
	auto& defs = cmd_in.folder.commanded->work.viewables;
	const auto project_path = cmd_in.folder.current_path;

	auto& work = cmd_in.folder.commanded->work;
	auto& cosm = work.world;

	singular_edit_properties<common_state_debugger_behaviour>(
		in,
		signi,
		" (Common state)",
		change_common_state_command(),
		special_widgets(
			pathed_asset_widget { defs, project_path, cmd_in },
			unpathed_asset_widget { defs, cosm.get_logical_assets() },
			flavour_widget { cosm }
		)
	);
}

void debugger_common_state_gui::perform(const debugger_settings& settings, const debugger_command_input in) {
	using namespace augs::imgui;

	auto window = make_scoped_window();
	
	if (!window) {
		return;
	}

	auto& work = in.folder.commanded->work;
	auto& cosm = work.world;

	ImGui::Columns(2); // 4-ways, with border
	next_column_text_disabled("Details");
	ImGui::Separator();

	edit_common(
		{ { settings.property_debugger, property_debugger_data }, in },
		cosm.get_common_significant()
	);
}
#else
void debugger_common_state_gui::perform(const debugger_settings& settings, const debugger_command_input in) {
	(void)settings;
	(void)in;
}
#endif
