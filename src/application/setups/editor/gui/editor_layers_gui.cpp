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

	const auto inspected_cols = std::array<rgba, 3> {
		rgba(35-10, 60-10, 90-10, 255),
		rgba(35, 60, 90, 255),
		rgba(35+10, 60+10, 90+10, 255)
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
			tint = rgba(255, 255, 255, 80);
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

	//const auto& style = ImGui::GetStyle();
	const auto disabled_color = rgba(255, 255, 255, 110);

	{
		//auto child = scoped_child("hierarchy view", ImVec2(0, -(ImGui::GetFrameHeightWithSpacing() + 4)));

		auto& layers = in.layers;

		int id_counter = 0;

		const auto avail = ImGui::GetContentRegionAvail().x;

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, avail - max_icon_size);

		for (const auto layer_id : layers.order) {
			auto* maybe_layer = in.setup.find_layer(layer_id);

			if (maybe_layer == nullptr) {
				continue;
			}

			auto& layer = *maybe_layer;
			const bool was_disabled = !layer.visible;

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

			const auto node_label = typesafe_sprintf("%x###HierarchyButton", layer.name);

			icon = in.necessary_images[visibility_icon];

			{
				const bool is_inspected = in.setup.is_inspected(layer_id);

				auto colored_selectable = scoped_selectable_colors(is_inspected ? inspected_cols : bg_cols);

				auto flags = 
					ImGuiTreeNodeFlags_OpenOnDoubleClick
					| ImGuiTreeNodeFlags_OpenOnArrow
					| ImGuiTreeNodeFlags_SpanAvailWidth
				;

				if (is_inspected) {
					flags |= ImGuiTreeNodeFlags_Selected;
				}

				if (was_disabled) {
					ImGui::PushStyleColor(ImGuiCol_Text, disabled_color.operator ImVec4());
				}

				if (const auto node = scoped_tree_node_ex(node_label.c_str(), flags)) {

				}

				if (was_disabled) {
					ImGui::PopStyleColor();
				}


				if (ImGui::IsItemClicked()) {
					in.setup.inspect(layer_id);

					if (ImGui::IsMouseDoubleClicked(0)) {
						//layer.gui_open = !layer.gui_open;
					}
				}

				if (ImGui::BeginDragDropSource()) {
					ImGui::SetDragDropPayload("editor_layer", nullptr, 0);
					text(label);
					ImGui::EndDragDropSource();
				}
			}

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("editor_layer"))
				{
					LOG("Dropped %x on layer", "layer");
				}

				ImGui::EndDragDropTarget();
			}

			ImGui::NextColumn();

			{
				const auto scaled_icon_size = vec2::scaled_to_max_size(icon.get_original_size(), max_icon_size);
				const auto cols = was_disabled ? colors_nha { disabled_color, disabled_color, disabled_color } : colors_nha{};

				if (game_image_button("###Visibility", icon, scaled_icon_size, cols, atlas_type)) {
					layer.visible = !layer.visible;
				}
			}

			ImGui::NextColumn();

		}
	}
}
