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

editor_filesystem_gui::editor_filesystem_gui(const std::string& name) : base(name) {
	setup_special_filesystem();
}

void editor_filesystem_gui::perform(const editor_project_files_input in) {
	using namespace augs::imgui;
	using namespace editor_widgets;

	(void)in;
	
	entity_to_highlight.unset();

	auto window = make_scoped_window();

	if (dragged_resource.is_set()) {
		const bool payload_still_exists = [&]() {
			const auto payload = ImGui::GetDragDropPayload();
			return payload && payload->IsDataType("dragged_resource");

		}();

		const bool mouse_over_scene = !mouse_over_any_window();

		if (!payload_still_exists) {
			if (previewed_created_node.is_set()) {
				in.setup.finish_moving_selection();
			}

			dragged_resource.unset();
			previewed_created_node.unset();
		}
		else {
			if (mouse_over_scene) {
				auto instantiate = [&]<typename T>(const T& typed_resource, const auto resource_id) {
					const auto resource_name = typed_resource.get_display_name();
					const auto new_name = in.setup.get_free_node_name_for(resource_name);

					using node_type = typename T::node_type;

					node_type new_node;
					new_node.resource_id = resource_id;
					new_node.unique_name = new_name;
					new_node.editable.pos = in.setup.get_world_cursor_pos();
					world_position_started_dragging = new_node.editable.pos;

					create_node_command<node_type> command;

					command.built_description = typesafe_sprintf("Created %x", new_name);
					command.created_node = std::move(new_node);

					const auto place_over_node = in.setup.get_topmost_inspected_node();
					entity_to_highlight = in.setup.to_entity_id(place_over_node);

					if (const auto parent_layer = in.setup.find_parent_layer(place_over_node)) {
						command.layer_id = parent_layer->layer_id;
						command.index_in_layer = parent_layer->index_in_layer;
					}
					else {
						command.create_layer = create_layer_command();
						command.create_layer->created_layer.unique_name = in.setup.get_free_layer_name();
					}

					const auto& executed = in.setup.post_new_command(std::move(command));

					in.setup.start_moving_selection();
					in.setup.make_last_command_a_child();
					in.setup.show_absolute_mover_pos_once();

					previewed_created_node = executed.get_node_id();

					in.setup.scroll_once_to(previewed_created_node);
				};

				if (!previewed_created_node.is_set()) {
					in.setup.on_resource(
						dragged_resource,
						instantiate
					);
				}
			}
			else {
				if (previewed_created_node.is_set()) {
					// LOG("Interrupting drag.");
					in.setup.finish_moving_selection();
					in.setup.undo_quiet();
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

	rebuild_special_filesystem(in);

	thread_local ImGuiTextFilter filter;
	filter_with_hint(filter, "##FilesFilter", "Search resources...");

	int id_counter = 0;

	if (filter.IsActive()) {
		in.files_root.reset_filter_flags();
		special_root.reset_filter_flags();

		auto& f = filter;

		auto filter_callback = [&f](auto& entry) {
			const auto path_in_project = entry.get_path_in_project();

			if (f.PassFilter(path_in_project.string().c_str())) {
				entry.mark_passed_filter();
			}
		};

		in.files_root.for_each_entry_recursive(filter_callback);
		special_root.for_each_entry_recursive(filter_callback);
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

	auto child = scoped_child(showing_official() ? "nodes official view" : "nodes project view");

	auto node_callback = [&](editor_filesystem_node& node) {
		auto id_scope = scoped_id(id_counter++);

		filesystem_node_widget(
			in.setup,
			node,
			editor_icon_info_in(in),
			filter.IsActive(),
			dragged_resource
		);
	};

	const bool with_closed_folders = filter.IsActive();
	in.files_root.in_ui_order(node_callback, with_closed_folders);

	ImGui::Separator();

	const bool special_resource_inspected = false;

	if (icon_button("##NewResource", in.necessary_images[assets::necessary_image_id::EDITOR_ICON_ADD], [](){}, "New special resource", !showing_official())) {

	}

	ImGui::SameLine();

	if (icon_button("##Duplicate", in.necessary_images[assets::necessary_image_id::EDITOR_ICON_CLONE], [](){}, "Duplicate selection", special_resource_inspected)) {

	}

	ImGui::SameLine();

	{
		const auto remove_bgs = std::array<rgba, 3> {
			rgba(0, 0, 0, 0),
			rgba(80, 20, 20, 255),
			rgba(150, 40, 40, 255)
		};

		const auto remove_tint = rgba(220, 80, 80, 255);

		if (icon_button("##Remove", in.necessary_images[assets::necessary_image_id::EDITOR_ICON_REMOVE], [](){}, "Remove selection", !showing_official() && special_resource_inspected, remove_tint, remove_bgs)) {

		}
	}

	ImGui::SameLine();

	text_disabled("(Special resources)");
	special_root.in_ui_order(node_callback, with_closed_folders);
}

void editor_filesystem_gui::clear_drag_drop() {
	ImGui::ClearDragDrop();
	augs::imgui::release_mouse_buttons();

	previewed_created_node.unset();
	dragged_resource.unset();
}

void editor_filesystem_gui::setup_special_filesystem() {
	special_root.clear();
	special_root.subfolders.resize(4);
	special_root.should_sort = false;

	auto i = 0;

	auto& lights_folder = special_root.subfolders[i++];
	auto& particles_folder = special_root.subfolders[i++];
	auto& wandering_pixels_folder = special_root.subfolders[i++];
	auto& prefabs_folder = special_root.subfolders[i++];

	lights_folder.name = "Lights";
	particles_folder.name = "Particles";
	wandering_pixels_folder.name = "Wandering pixels";
	prefabs_folder.name = "Prefabs";

	for (auto& s : special_root.subfolders) {
		s.type = editor_filesystem_node_type::FOLDER;
	}

	special_root.adding_children_finished();
}

void editor_filesystem_gui::rebuild_special_filesystem(const editor_project_files_input in) {
	auto i = 0;

	const auto& resources = showing_official() ? in.setup.get_official_resources() : in.setup.get_project().resources;

	auto& lights_folder = special_root.subfolders[i++];
	auto& particles_folder = special_root.subfolders[i++];
	auto& wandering_pixels_folder = special_root.subfolders[i++];
	auto& prefabs_folder = special_root.subfolders[i++];

	auto handle = [&]<typename P>(editor_filesystem_node& parent, P& pool, const auto icon_id) {
		using resource_type = typename P::value_type;

		parent.game_atlas_icon = in.necessary_images[icon_id];
		parent.after_text = typesafe_sprintf("(%x)", pool.size());

		parent.files.resize(pool.size());
		std::size_t i = 0;

		auto resource_handler = [&](const auto raw_id, const resource_type& typed_resource) {
			editor_resource_id resource_id;

			resource_id.is_official = showing_official();
			resource_id.raw = raw_id;
			resource_id.type_id.set<resource_type>();

			auto& new_node = parent.files[i++];
			new_node.name = typed_resource.unique_name;
			new_node.associated_resource = resource_id;
			new_node.type = editor_filesystem_node_type::OTHER_RESOURCE;
			new_node.game_atlas_icon = parent.game_atlas_icon;
		};

		pool.for_each_id_and_object(resource_handler);
		parent.adding_children_finished();
	};

	// TODO: redirect to proper pools!
	handle(particles_folder, resources.get_pool_for<editor_light_resource>(), assets::necessary_image_id::EDITOR_ICON_PARTICLE_SOURCE);
	handle(wandering_pixels_folder, resources.get_pool_for<editor_light_resource>(), assets::necessary_image_id::EDITOR_ICON_WANDERING_PIXELS);
	handle(prefabs_folder, resources.get_pool_for<editor_light_resource>(), assets::necessary_image_id::EDITOR_ICON_BOMBSITE_A);

	handle(lights_folder, resources.get_pool_for<editor_light_resource>(), assets::necessary_image_id::EDITOR_ICON_LIGHT);

	special_root.set_parents(0);
}
