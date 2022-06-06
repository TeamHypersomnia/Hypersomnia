#pragma once
#include "game/detail/render_layer_filter.h"

#include "augs/misc/imgui/standard_window_mixin.h"
#include "application/setups/debugger/property_debugger/property_debugger_settings.h"

struct debugger_layers_gui : standard_window_mixin<debugger_layers_gui> {
	using base = standard_window_mixin<debugger_layers_gui>;
	using base::base;
	using introspect_base = base;

	void perform(
		const property_debugger_settings& settings,
		augs::maybe<render_layer_filter>& viewing,
		augs::maybe<render_layer_filter>& selecting
	);
};
