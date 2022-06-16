#pragma once
#include "augs/misc/imgui/standard_window_mixin.h"
#include "application/setups/editor/nodes/editor_node_id.h"
#include "application/setups/editor/resources/editor_resource_id.h"

using inspected_variant = std::variant<
	editor_node_id,
	editor_resource_id
>;

struct editor_inspector_input {
	editor_project& project;
};

struct editor_inspector_gui : standard_window_mixin<editor_inspector_gui> {
	using base = standard_window_mixin<editor_inspector_gui>;
	using base::base;
	using introspect_base = base;

	inspected_variant currently_inspected;

	void perform(editor_inspector_input);
};

