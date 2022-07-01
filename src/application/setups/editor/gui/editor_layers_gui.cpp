#include "augs/templates/history.hpp"
#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_controls.h"
#include "augs/misc/imgui/imgui_game_image.h"

#include "application/setups/editor/editor_filesystem.h"

#include "application/setups/editor/project/editor_layers.h"
#include "application/setups/debugger/detail/maybe_different_colors.h"
#include "application/setups/editor/gui/editor_layers_gui.h"
#include "application/setups/editor/project/editor_project.h"
#include "application/setups/editor/editor_setup.h"

void editor_layers_gui::perform(const editor_layers_input in) {
	using namespace augs::imgui;

	(void)in;
	
	auto window = make_scoped_window();

	if (!window) {
		return;
	}

	const auto max_icon_size = ImGui::GetTextLineHeight();

	const auto now_inspected = in.setup.get_inspected();
	const auto inspected_node = std::get_if<editor_node_id>(std::addressof(now_inspected));
	(void)inspected_node;

	const bool node_or_layer_inspected = false;//inspected_node != nullptr;

	thread_local ImGuiTextFilter filter;

	const auto bg_cols = std::array<rgba, 3> {
		rgba(0, 0, 0, 0),
		rgba(15, 40, 70, 255),
		rgba(35, 60, 90, 255)
	};

	auto icon_button = [&](
		const auto label, 
		const auto icon_id, 
		const std::string& tooltip,
		const bool enabled,
		rgba tint = white, 
		std::array<rgba, 3> icon_bg_cols = {
			rgba(0, 0, 0, 0),
			rgba(35, 60, 90, 255),
			rgba(35+10, 60+10, 90+10, 255)
		}
	) {
		auto id = scoped_id(label);

		if (!enabled) {
			tint = rgba(255, 255, 255, 50);
		}

		const auto icon = in.necessary_images[icon_id];
		const auto scaled_icon_size = vec2::scaled_to_max_size(icon.get_original_size(), max_icon_size);

		bool result = false;

		{
			auto preserved = scoped_preserve_cursor();

			auto colored_selectable = scoped_selectable_colors(icon_bg_cols);

			{
				auto scope = maybe_disabled_cols({}, !enabled);
				result = ImGui::Selectable("##ButtonBg", false, ImGuiSelectableFlags_None, ImVec2(scaled_icon_size));
			}

			if (ImGui::IsItemHovered()) {
				auto scope = scoped_tooltip();
				text_color(tooltip, tint);
			}
		}

		game_image(icon, scaled_icon_size, tint);
		return result;
	};

	if (icon_button("##NewLayer", assets::necessary_image_id::EDITOR_ICON_ADD, "New layer", true)) {
		in.setup.create_new_layer();
	}

	ImGui::SameLine();

	if (icon_button("##Duplicate", assets::necessary_image_id::EDITOR_ICON_CLONE, "Duplicate selection", node_or_layer_inspected)) {

	}

	ImGui::SameLine();

	{
		const auto remove_bgs = std::array<rgba, 3> {
			rgba(0, 0, 0, 0),
			rgba(80, 20, 20, 255),
			rgba(150, 40, 40, 255)
		};

		const auto remove_tint = rgba(220, 80, 80, 255);

		if (icon_button("##Remove", assets::necessary_image_id::EDITOR_ICON_REMOVE, "Remove selection", node_or_layer_inspected, remove_tint, remove_bgs)) {

		}
	}
	
	ImGui::SameLine();

	acquire_keyboard_once();
	filter_with_hint(filter, "##HierarchyFilter", "Search objects and layers...");

	{
		//auto child = scoped_child("hierarchy view", ImVec2(0, -(ImGui::GetFrameHeightWithSpacing() + 4)));

		auto& layers = in.layers;

		int id_counter = 0;

		for (const auto layer_id : layers.order) {
			auto* maybe_layer = in.setup.find_layer(layer_id);

			if (maybe_layer == nullptr) {
				continue;
			}

			auto& layer = *maybe_layer;

			auto id_scope = scoped_id(id_counter++);
			const auto label = layer.name;

			using namespace augs::imgui;
			augs::atlas_entry icon;

			if (filter.IsActive()) {
				if (!layer.passed_filter) {
					return;
				}
			}

			auto atlas_type = augs::imgui_atlas_type::GAME;

			const auto visibility_icon = layer.visible
				? assets::necessary_image_id::EDITOR_ICON_VISIBLE
				: assets::necessary_image_id::EDITOR_ICON_HIDDEN
			;

			const auto label_color = layer.visible ? white : rgba(255, 255, 255, 180);

			text_color(layer.name, label_color);

			icon = in.necessary_images[visibility_icon];

			const float size_mult = 1.1f;
			const auto text_h = ImGui::GetTextLineHeight();
			const auto button_size = ImVec2(0, text_h * size_mult);

			const auto before_pos = ImGui::GetCursorPos();

			bool result = false;

			{
				auto colored_selectable = scoped_selectable_colors(bg_cols);
				auto id = scoped_id(label.c_str());

				result = ImGui::Selectable("###HierarchyButton", false, ImGuiSelectableFlags_DrawHoveredWhenHeld, button_size);

				if (result) {
					layer.gui_open = !layer.gui_open;
				}

				if (ImGui::BeginDragDropSource()) {
					ImGui::SetDragDropPayload("editor_layer", nullptr, 0);
					text(label);
					ImGui::EndDragDropSource();
				}
			}

			const auto after_pos = ImGui::GetCursorPos();

			const int node_level = 0;

			{
				auto scope = scoped_preserve_cursor();

				const float content_x_offset = max_icon_size * node_level;

				const auto icon_size = vec2::square(max_icon_size);
				const auto scaled_icon_size = vec2::scaled_to_max_size(icon.get_original_size(), max_icon_size);
				const auto diff = (icon_size - scaled_icon_size) / 2;

				ImGui::SetCursorPos(ImVec2(vec2(before_pos) + vec2(content_x_offset, 0) + diff));

				const auto icon_padding = vec2(icon_size) / 1.5f;

				game_image(icon, scaled_icon_size, white, vec2::zero, atlas_type);

				const auto image_offset = vec2(0, button_size.y / 2 - icon_size.y / 2);
				const auto text_pos = vec2(before_pos) + image_offset + vec2(content_x_offset + icon_size.x + icon_padding.x, icon_size.y / 2 - text_h / 2);
				ImGui::SetCursorPos(ImVec2(text_pos));
				text_color(label, label_color);
			}

			ImGui::SetCursorPos(after_pos);

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("editor_layer"))
				{
					LOG("Dropped %x on layer", "layer");
				}

				ImGui::EndDragDropTarget();
			}
		}
	}
}
