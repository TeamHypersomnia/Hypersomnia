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
			return payload && payload->IsDataType("dragged_resource");

		}();

		const bool mouse_over_scene = !mouse_over_any_window();

		if (!payload_still_exists) {
			dragged_resource = nullptr;
			previewed_created_node.unset();
		}
		else {
			if (mouse_over_scene) {
				auto instantiate = [&]<typename T>(const T& typed_resource, const auto resource_id) {
					if (previewed_created_node.is_set()) {
						in.setup.undo_quiet();
						in.setup.rebuild_scene();
					}
					else {
						// LOG("Start dropping resource on scene");
					}

					const auto resource_name = typed_resource.get_display_name();
					const auto new_name = in.setup.get_free_node_name_for(resource_name);

					using node_type = typename T::node_type;

					node_type new_node;
					new_node.resource_id = resource_id;
					new_node.unique_name = new_name;
					new_node.editable.pos = in.setup.get_world_cursor_pos();

					create_node_command<node_type> command;

					command.built_description = typesafe_sprintf("Created %x", new_name);
					command.created_node = std::move(new_node);

					const auto hovered_node = in.setup.get_hovered_node();

					if (const auto parent_layer = in.setup.find_parent_layer(hovered_node)) {
						command.layer_id = parent_layer->first;
						command.index_in_layer = parent_layer->second;
					}
					else {
						command.create_layer = create_layer_command();
						command.create_layer->chosen_name = in.setup.get_free_layer_name();
					}

					const auto& executed = in.setup.post_new_command(std::move(command));
					previewed_created_node = executed.get_node_id();
				};

				in.setup.on_resource(
					dragged_resource->associated_resource,
					instantiate
				);
			}
			else {
				if (previewed_created_node.is_set()) {
					// LOG("Interrupting drag.");
					in.setup.undo_quiet();
					in.setup.rebuild_scene();
					previewed_created_node.unset();
				}
			}
		}
	}
	else {
		previewed_created_node.unset();
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

		if (node.parent == std::addressof(in.files_root)) {
			/* Ignore some editor-specific files in the project folder */

			if (node.name == "editor_view.json") {
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

					ImGui::SetDragDropPayload("dragged_resource", nullptr, 0);
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

void editor_filesystem_gui::clear_drag_drop() {
	ImGui::ClearDragDrop();
	augs::imgui::release_mouse_buttons();

	previewed_created_node.unset();
	clear_pointers();
}
