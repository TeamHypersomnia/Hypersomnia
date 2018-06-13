#pragma once
#include <tuple>
#include "3rdparty/imgui/imgui.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"

inline auto in_line_button_style() {
	using namespace augs::imgui;

	return std::make_tuple(
		scoped_style_var(ImGuiStyleVar_ItemSpacing, ImVec2(3, 1)),
		scoped_style_var(ImGuiStyleVar_FramePadding, ImVec2(3, 1))
	);
}
