#include "augs/templates/history.hpp"
#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_controls.h"
#include "augs/misc/imgui/imgui_game_image.h"
#include "application/setups/editor/editor_filesystem.h"

#include "application/setups/editor/gui/editor_filesystem_gui.h"
#include "application/setups/editor/editor_setup.hpp"
#include "application/setups/editor/gui/widgets/inspectable_with_icon.h"
#include "application/setups/editor/gui/widgets/icon_button.h"
#include "application/setups/editor/gui/widgets/filesystem_node_widget.h"

void editor_filesystem_gui::perform(const editor_project_files_input in) {
	using namespace augs::imgui;
	using namespace editor_widgets;

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

					in.setup.scroll_once_to(previewed_created_node);
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
	filter_with_hint(filter, "##FilesFilter", "Search resources...");

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

	{
		const auto bg_cols = std::array<rgba, 3> {
			rgba(0, 0, 0, 0),
			rgba(15, 40, 70, 255),
			rgba(35, 60, 90, 255)
		};

		const auto selected_cols = std::array<rgba, 3> {
			rgba(35-10, 60-10, 90-10, 255),
			rgba(35, 60, 90, 255),
			rgba(35+10, 60+10, 90+10, 255)
		};

		const auto avail = ImGui::GetContentRegionAvail();
		auto spacing = scoped_style_var(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

		auto tab_button = [&](const auto label, const auto type) {
			const bool selected = current_tab == type;
			auto cols = scoped_button_colors(selected ? selected_cols : bg_cols);

			if (ImGui::Button(label, ImVec2(avail.x / 2, 0))) {
				current_tab = type;
			}
		};

		tab_button("Project", editor_resources_tab_type::PROJECT);
		ImGui::SameLine();
		tab_button("Official", editor_resources_tab_type::OFFICIAL);
	}

	auto node_callback = [&](editor_filesystem_node& node) {
		auto id_scope = scoped_id(id_counter++);

		filesystem_node_widget(
			in.setup,
			node,
			in.necessary_images,
			in.ad_hoc_atlas,
			filter.IsActive(),
			dragged_resource
		);
	};

	const bool with_closed_folders = filter.IsActive();
	in.files_root.in_ui_order(node_callback, with_closed_folders);

	auto atlas_type = augs::imgui_atlas_type::GAME;

	ImGui::Separator();

	const bool special_resource_inspected = false;
	const bool is_official = current_tab == editor_resources_tab_type::OFFICIAL;

	if (icon_button("##NewResource", in.necessary_images[assets::necessary_image_id::EDITOR_ICON_ADD], "New special resource", !is_official)) {

	}

	ImGui::SameLine();

	if (icon_button("##Duplicate", in.necessary_images[assets::necessary_image_id::EDITOR_ICON_CLONE], "Duplicate selection", special_resource_inspected)) {

	}

	ImGui::SameLine();

	{
		const auto remove_bgs = std::array<rgba, 3> {
			rgba(0, 0, 0, 0),
			rgba(80, 20, 20, 255),
			rgba(150, 40, 40, 255)
		};

		const auto remove_tint = rgba(220, 80, 80, 255);

		if (icon_button("##Remove", in.necessary_images[assets::necessary_image_id::EDITOR_ICON_REMOVE], "Remove selection", !is_official && special_resource_inspected, remove_tint, remove_bgs)) {

		}
	}

	ImGui::SameLine();

	text_disabled("(Special resources)");

	auto after_text = [](const int num_resources) {
		return typesafe_sprintf("(%x)", num_resources);
	};

	const auto& special_resources_source = is_official ? in.setup.get_official_resources() : in.setup.get_project().resources;

	const auto num_lights = after_text(special_resources_source.get_pool_for<editor_light_resource>().size());
	const auto num_particles = after_text(0);
	const auto num_wandering_pixels = after_text(0);
	const auto num_prefabs = after_text(0);

	inspectable_with_icon(in.necessary_images[assets::necessary_image_id::EDITOR_ICON_LIGHT], atlas_type, "Lights", white, 0, false, [](){}, num_lights);
	inspectable_with_icon(in.necessary_images[assets::necessary_image_id::EDITOR_ICON_PARTICLE_SOURCE], atlas_type, "Particles", white, 0, false, [](){}, num_particles);
	inspectable_with_icon(in.necessary_images[assets::necessary_image_id::EDITOR_ICON_WANDERING_PIXELS], atlas_type, "Wandering pixels", white, 0, false, [](){}, num_wandering_pixels);
	inspectable_with_icon(in.necessary_images[assets::necessary_image_id::EDITOR_ICON_BOMBSITE_A], atlas_type, "Prefabs", white, 0, false, [](){}, num_prefabs);
}

void editor_filesystem_gui::clear_drag_drop() {
	ImGui::ClearDragDrop();
	augs::imgui::release_mouse_buttons();

	previewed_created_node.unset();
	clear_pointers();
}
