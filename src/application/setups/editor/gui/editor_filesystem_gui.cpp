#include "augs/templates/history.hpp"
#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_controls.h"
#include "augs/misc/imgui/imgui_game_image.h"
#include "application/setups/editor/editor_filesystem.h"

#include "application/setups/editor/gui/editor_filesystem_gui.h"
#include "application/setups/editor/editor_setup.hpp"

void editor_filesystem_gui::perform(const editor_project_files_input in) {
	using namespace augs::imgui;

	(void)in;
	
	auto window = make_scoped_window();

	if (dragged_resource != nullptr) {
		const bool payload_still_exists = [&]() {
			const auto payload = ImGui::GetDragDropPayload();
			return payload && payload->IsDataType("editor_filesystem_gui");

		}();

		if (!payload_still_exists) {
			const bool mouse_over_scene = !mouse_over_any_window();

			if (mouse_over_scene) {
				LOG("Dropped %x on scene", dragged_resource->name);

				dragged_resource = nullptr;
			}
		}
	}

	if (!window) {
		return;
	}

	thread_local ImGuiTextFilter filter;
	{
		ImGui::SetNextItemWidth(-0.0001f);
		bool value_changed = ImGui::InputTextWithHint("##FilesFilter", "Search project files...", filter.InputBuf, IM_ARRAYSIZE(filter.InputBuf));
		if (value_changed)
		filter.Build();
	}

	int id_counter = 0;

	auto node_callback = [&](editor_filesystem_node& node) {
		using namespace augs::imgui;
		augs::atlas_entry icon;

		auto id_scope = scoped_id(id_counter++);

		const auto max_icon_size = ImGui::GetTextLineHeight();
		auto atlas_type = augs::imgui_atlas_type::GAME;

		if (node.is_folder()) {
			icon = in.necessary_images[assets::necessary_image_id::EDITOR_ICON_FOLDER];
		}
		else {
			if (auto ad_hoc = mapped_or_nullptr(in.ad_hoc_atlas, node.file_thumbnail_id)) {
				icon = *ad_hoc;
				atlas_type = augs::imgui_atlas_type::AD_HOC;
			}
			else {
				icon = in.necessary_images[assets::necessary_image_id::EDITOR_ICON_CREATE];
			}
		}

		const auto& label = node.name;
		const auto label_color = white;
		const auto bg_cols = std::array<rgba, 3> {
			rgba(0, 0, 0, 0),
			rgba(15, 40, 70, 255),
			rgba(35, 60, 90, 255)
		};

		const float size_mult = 1.1f;
		//const float padding_mult = 0.1f;

		const auto text_h = ImGui::GetTextLineHeight();
		const auto button_size = ImVec2(0, text_h * size_mult);

		//shift_cursor(vec2(0, text_h * padding_mult));

		const auto before_pos = ImGui::GetCursorPos();

		bool result = false;

		{
			auto colored_selectable = scoped_selectable_colors(
				bg_cols[0],
				bg_cols[1],
				bg_cols[2]
			);

			auto id = scoped_id(label.c_str());

			result = ImGui::Selectable("###Button", false, ImGuiSelectableFlags_None, button_size);

			if (node.is_resource()) {
				if (ImGui::BeginDragDropSource())
				{
					dragged_resource = std::addressof(node);

					ImGui::SetDragDropPayload("editor_filesystem_gui", nullptr, 0);
					text(label);
					ImGui::EndDragDropSource();
				}
			}
		}

		const auto after_pos = ImGui::GetCursorPos();

		{
			auto scope = scoped_preserve_cursor();

			const float content_x_offset = max_icon_size * node.level;

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

		//shift_cursor(vec2(0, text_h * padding_mult));

		if (result) {
			node.toggle_open();
		}
	};

	in.files_root.in_ui_order(node_callback);
}
