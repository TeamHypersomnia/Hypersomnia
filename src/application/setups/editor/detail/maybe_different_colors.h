#pragma once
#include <tuple>
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "application/setups/editor/gui/property_editor/property_editor_settings.h"

inline auto maybe_different_value_cols(
	const property_editor_settings& settings,
	const bool values_different
) {
	using namespace augs::imgui;

	return std::make_tuple(
		cond_scoped_style_color(values_different, ImGuiCol_FrameBg, settings.different_values_frame_bg),
		cond_scoped_style_color(values_different, ImGuiCol_FrameBgHovered, settings.different_values_frame_hovered_bg),
		cond_scoped_style_color(values_different, ImGuiCol_FrameBgActive, settings.different_values_frame_active_bg)
	);
};

inline auto maybe_disabled_cols(
	const bool are_disabled
) {
	using namespace augs::imgui;
	const auto disabled_col = rgba(10, 10, 10, 255);

	return std::make_tuple(
		cond_scoped_item_flag(are_disabled, ImGuiItemFlags_Disabled, true),
		cond_scoped_style_color(are_disabled, ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled)),
		cond_scoped_style_color(are_disabled, ImGuiCol_Button, disabled_col),
		cond_scoped_style_color(are_disabled, ImGuiCol_ButtonHovered, disabled_col),
		cond_scoped_style_color(are_disabled, ImGuiCol_ButtonActive, disabled_col),
		cond_scoped_style_color(are_disabled, ImGuiCol_FrameBg, disabled_col),
		cond_scoped_style_color(are_disabled, ImGuiCol_FrameBgHovered, disabled_col),
		cond_scoped_style_color(are_disabled, ImGuiCol_FrameBgActive, disabled_col)
	);
};

inline auto maybe_disabled_only_cols(
	const bool are_disabled
) {
	using namespace augs::imgui;
	const auto disabled_col = rgba(10, 10, 10, 255);

	return std::make_tuple(
		cond_scoped_style_color(are_disabled, ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled)),
		cond_scoped_style_color(are_disabled, ImGuiCol_Button, disabled_col),
		cond_scoped_style_color(are_disabled, ImGuiCol_ButtonHovered, disabled_col),
		cond_scoped_style_color(are_disabled, ImGuiCol_ButtonActive, disabled_col),
		cond_scoped_style_color(are_disabled, ImGuiCol_FrameBg, disabled_col),
		cond_scoped_style_color(are_disabled, ImGuiCol_FrameBgHovered, disabled_col),
		cond_scoped_style_color(are_disabled, ImGuiCol_FrameBgActive, disabled_col)
	);
};
