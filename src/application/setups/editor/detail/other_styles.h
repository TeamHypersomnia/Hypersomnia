#pragma once
#include "3rdparty/imgui/imgui.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"

inline auto in_line_button_style() {
	using namespace augs::imgui;

	return scoped_style_var(ImGuiStyleVar_FramePadding, ImVec2(3, 1));
}
