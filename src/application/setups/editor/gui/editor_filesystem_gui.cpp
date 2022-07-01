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

	acquire_keyboard_once();

	thread_local ImGuiTextFilter filter;
	filter_with_hint(filter, "##FilesFilter", "Search project files...");

	int id_counter = 0;

	if (filter.IsActive()) {
		in.files_root.reset_filter_flags();

		auto& f = filter;

		in.files_root.for_each_entry_recursive(
			[&f](auto& entry) {
				const auto path_in_project = entry.get_path_in_project();

				if (f.PassFilter(path_in_project.string().c_str())) {
					entry.mark_passed_filter();
				}
			}
		);
	}

	auto node_callback = [&](editor_filesystem_node& node) {
		using namespace augs::imgui;
		augs::atlas_entry icon;

		if (filter.IsActive()) {
			if (!node.passed_filter) {
				return;
			}
		}

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
				icon = in.necessary_images[assets::necessary_image_id::EDITOR_ICON_FILE];
			}
		}

		const bool is_inspected = in.setup.is_inspected(node.associated_resource);

		const auto& label = node.name;
		const auto label_color = white;
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

		const float size_mult = 1.1f;
		//const float padding_mult = 0.1f;

		const auto text_h = ImGui::GetTextLineHeight();
		const auto button_size = ImVec2(0, text_h * size_mult);

		//shift_cursor(vec2(0, text_h * padding_mult));

		const auto before_pos = ImGui::GetCursorPos();

		bool result = false;

		{
			auto colored_selectable = scoped_selectable_colors(is_inspected ? inspected_cols : bg_cols);
			auto id = scoped_id(label.c_str());

			result = ImGui::Selectable("###Button", is_inspected, ImGuiSelectableFlags_DrawHoveredWhenHeld, button_size);

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

		//shift_cursor(vec2(0, text_h * padding_mult));

		if (result) {
			if (node.is_folder()) {
				if (!filter.IsActive()) {
					node.toggle_open();
				}
			}
			else {
				if (in.setup.exists(node.associated_resource)) {
					in.setup.inspect(node.associated_resource);
				}
			}
		}
	};

	const bool with_closed_folders = filter.IsActive();
	in.files_root.in_ui_order(node_callback, with_closed_folders);
}
