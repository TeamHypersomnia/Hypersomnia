#include "game/cosmos/logic_step.h"
#include "game/organization/all_messages_includes.h"

#include "view/viewables/viewables_loading_type.h"

#include "application/config_lua_table.h"
#include "application/setups/editor/editor_setup.h"

#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/window_framework/window.h"
#include "3rdparty/imgui/imgui_internal.h"
#include "application/setups/client/https_file_uploader.h"

void editor_setup::perform_main_menu_bar(const perform_custom_imgui_input in) {
	using namespace augs::imgui;

	if (miniature_generator.has_value()) {
		if (miniature_generator->complete()) {
			if (miniature_generator->reveal_when_complete) {
				in.window.reveal_in_explorer(miniature_generator->output_path);
				recent_message.set("Saved arena screenshot to:\n%x", miniature_generator->output_path);
				recent_message.show_for_at_least_ms = 10000;
			}
			else {
				recent_message.set("Saved arena miniature to\n%x", miniature_generator->output_path);
				recent_message.show_for_at_least_ms = 10000;
			}

			miniature_generator.reset();
		}
	}

	if (is_playtesting() && gui.playtest_immersive) {
		return;
	}

	(void)in;

	auto window_border_size = scoped_style_var(ImGuiStyleVar_WindowBorderSize, 0.0f);
	auto frame_border_size = scoped_style_var(ImGuiStyleVar_FrameBorderSize, 0.0f);

	auto item_if_tabs_and = [&](const bool condition, const char* label, const char* shortcut = nullptr) {
		return ImGui::MenuItem(label, shortcut, nullptr, condition);
	};

	auto item_if_tabs = [&](const char* label, const char* shortcut = nullptr) {
		return item_if_tabs_and(true, label, shortcut);
	};

	if (auto main_menu = scoped_menu_bar()) {
		const auto file_button_label = get_arena_name_with_star();

		auto item_if = [](const bool condition, const char* label, const char* shortcut = nullptr) {
			return ImGui::MenuItem(label, shortcut, nullptr, condition);
		};

		if (auto menu = scoped_menu(file_button_label.c_str())) {
			if (ImGui::MenuItem("Save", "CTRL+S")) {
				save();
			}

			if (ImGui::MenuItem("Reveal in explorer", "CTRL+SHIFT+E")) {
				in.window.reveal_in_explorer(paths.project_json);
			}

			ImGui::Separator();

			if (item_if(upload_icon_visible(), "Upload arena", "CTRL+SHIFT+U")) {
				upload_to_arena_server();
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Go back to Projects list")) {
				imgui_return_once = custom_imgui_result::GO_TO_PROJECT_SELECTOR;
			}
		}

		if (auto menu = scoped_menu("Edit")) {
			if (item_if(can_undo(), "Undo", "CTRL+Z")) {
				undo();
			}

			if (item_if(can_redo(), "Redo", "CTRL+SHIFT+Z")) {
				redo();
			}

			ImGui::Separator();

			const bool any_node_selected = inspects_any<editor_node_id>();

			if (item_if(any_node_selected, "Move", "T")) {
				warp_cursor_to_center(in.window);
				start_moving_selection();
			}

			if (item_if(any_node_selected, "Rotate", "R")) {
				start_rotating_selection();
			}

			const bool can_resize = can_resize_selected_nodes();

			if (ImGui::BeginMenu("Resize (E)", can_resize)) {
				auto do_resize_opt = [&](
					const auto label,
					const bool two,
					const auto edges,
					const auto edge_icon_angle
				) {
					(void)edge_icon_angle;

					if (ImGui::MenuItem(label)) {
						start_resizing_selection(two, edges);
					}
				};

				using ae = resize_nodes_command::active_edges;

				do_resize_opt("Drag top edge", false, ae(true, false, false, false), -90);
				do_resize_opt("Drag left edge", false, ae(false, true, false, false), -180);
				do_resize_opt("Drag right edge", false, ae(false, false, true, false), 0);
				do_resize_opt("Drag bottom edge", false, ae(false, false, false, true), 90);

				ImGui::Separator();

				do_resize_opt("Drag top-left edges", true, ae(true, true, false, false), -90);
				do_resize_opt("Drag top-right edges", true, ae(true, false, true, false), 0);
				do_resize_opt("Drag bottom-right edges", true, ae(false, false, true, true), 90);
				do_resize_opt("Drag bottom-left edges", true, ae(false, true, false, true), 180);

				ImGui::EndMenu();
			}

			if (item_if(any_node_selected, "Rotate 90* CCW", "SHIFT+R")) {
				rotate_selection_once_by(-90);
			}

			if (item_if(any_node_selected, "Rotate 90* CW", "CTRL+R")) {
				rotate_selection_once_by(90);
			}

			if (item_if(any_node_selected, "Reset rotation", "W")) {
				reset_rotation_of_selected_nodes();
			}

			if (item_if(any_node_selected, "Flip horizontally", "SHIFT+H")) {
				flip_selection_horizontally();
			}

			if (item_if(any_node_selected, "Flip vertically", "SHIFT+V")) {
				flip_selection_vertically();
			}

			ImGui::Separator();

			const bool node_or_layer_inspected = 
				inspects_any<editor_node_id>()
				|| inspects_any<editor_layer_id>()
			;

			if (item_if(node_or_layer_inspected, "Clone", "C")) {
				if (should_warp_cursor_before_cloning()) {
					warp_cursor_to_center(in.window);
				}

				clone_selection();
			}

			const bool can_mirror = node_or_layer_inspected;

			if (ImGui::BeginMenu("Mirror", can_mirror)) {
				auto do_mirror_opt = [&](const auto label, const auto shortcut, const auto dir, const auto arrow_icon_angle) {
					(void)arrow_icon_angle;

					if (ImGui::MenuItem(label, shortcut)) {
						mirror_selection(dir);
					}
				};

				do_mirror_opt("Up ", "CTRL+UP", vec2i(0, -1), -90);
				do_mirror_opt("Left", "CTRL+LEFT", vec2i(-1, 0), -180);
				do_mirror_opt("Right", "CTRL+RIGHT", vec2i(1, 0), 0);
				do_mirror_opt("Down", "CTRL+DOWN", vec2i(0, 1), 90);

				ImGui::EndMenu();
			}

			{
				const auto remove_bgs = std::array<rgba, 3> {
					rgba(0, 0, 0, 0),
					rgba(80, 20, 20, 255),
					rgba(150, 40, 40, 255)
				};

				const auto remove_tint = rgba(220, 80, 80, 255);

				auto colored_selectable = scoped_selectable_colors(remove_bgs);
				auto colored_col = scoped_text_color(remove_tint);

				if (item_if(node_or_layer_inspected, "Delete", "D")) {
					delete_selection();
				}
			}

			ImGui::Separator();

			if (item_if(true, "Project settings", "CTRL+U")) {
				inspect_project_settings();
			}
		}

		if (auto menu = scoped_menu("Window")) {
			auto do_window_entry = [&](auto& win, const auto shortcut) {
				const auto s = std::string(shortcut);
				if (item_if_tabs(win.get_title().c_str(), s.c_str())) {
					win.open();
				}
			};

			do_window_entry(gui.layers, "CTRL+L");
			do_window_entry(gui.filesystem, "CTRL+F");
			do_window_entry(gui.inspector, "CTRL+I");
			do_window_entry(gui.history, "CTRL+H");
		}

		if (auto menu = scoped_menu("View")) {
			if (ImGui::MenuItem("Toolbar", "CTRL+T", gui.toolbar.show, true)) {
				gui.toolbar.toggle();
			}

			if (ImGui::MenuItem("Sounds preview", "U", gui.sounds_preview, true)) {
				toggle_sounds_preview();
			}

			const bool grid_enabled = is_grid_enabled();
			const bool snapping_enabled = is_snapping_enabled();

			ImGui::Separator();

			if (ImGui::MenuItem("Grid", "G", grid_enabled, true)) {
				toggle_grid();
			}

			if (ImGui::MenuItem("Sparser grid", "[")) {
				sparser_grid();
			}

			if (ImGui::MenuItem("Denser grid", "]")) {
				denser_grid();
			}

			if (ImGui::MenuItem("Snap to grid", "S", snapping_enabled, true)) {
				toggle_snapping();
			}

			ImGui::Separator();

			const bool is_centered = is_view_centered_at_selection();

			if (item_if(!is_centered, "Focus", "F")) {
				center_view_at_selection();
			}

			const auto zoom = get_camera_eye().zoom;
			const bool nonzero_zoom = zoom != 1.0f;

			if (item_if(nonzero_zoom, "Reset zoom", "Z or =")) {
				reset_zoom();
			}
		}
	}
}

custom_imgui_result editor_setup::perform_custom_imgui(const perform_custom_imgui_input in) {
	(void)in;

	advance_uploading();
	do_uploading_imgui();

	arena_gui_base::perform_custom_imgui(in);

	auto update_autosave = [this]() {
		if (last_autosave_settings.interval_changed(settings.autosave)) {
			autosave_timer.reset();
		}

		last_autosave_settings = settings.autosave;

		if (settings.autosave.periodically) {
			if (!autosave_needed()) {
				autosave_timer.reset();
			}

			if (settings.autosave.once_every_min <= autosave_timer.get<std::chrono::minutes>()) {
				autosave_now_if_needed();
			}
		}
	};

	update_autosave();

	if (gui.request_toggle_sounds_preview) {
		gui.request_toggle_sounds_preview = false;

		toggle_sounds_preview();
	}

	if (is_playtesting() && gui.playtest_immersive) {
		return custom_imgui_result::NONE;
	}

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	window_flags |= ImGuiWindowFlags_NoBackground;

	{
		const auto viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin("DockSpace Demo", nullptr, window_flags);

	ImGui::PopStyleVar(3);

	{
		const auto dockspace_id = ImGui::GetID("MyDockSpace");

		auto make_default_layout = [&]() {
			ImGui::DockBuilderRemoveNode(dockspace_id); // Clear out existing layout
			ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace); // Add empty node
			ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetContentRegionAvail());

			ImGuiID dock_main_id = dockspace_id; // This variable will track the document node, however we are not using it here as we aren't docking anything into it.

			ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.20f, NULL, &dock_main_id);
			ImGuiID dock_id_bottom_right = ImGui::DockBuilderSplitNode(dock_id_right, ImGuiDir_Down, 0.30f, NULL, &dock_id_right);

			ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.25f, NULL, &dock_main_id);
			ImGuiID dock_id_bottom_left = ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Down, 0.40f, NULL, &dock_id_left);

			ImGuiID dock_id_top = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Up, 0.20f, NULL, &dock_main_id);

			ImGui::DockBuilderDockWindow(gui.filesystem.get_title().c_str(), dock_id_bottom_left);
			ImGui::DockBuilderDockWindow(gui.layers.get_title().c_str(), dock_id_left);
			ImGui::DockBuilderDockWindow(gui.inspector.get_title().c_str(), dock_id_right);
			ImGui::DockBuilderDockWindow(gui.history.get_title().c_str(), dock_id_bottom_right);

			ImGui::DockBuilderDockWindow(gui.toolbar.get_title().c_str(), dock_id_top);

			ImGui::DockBuilderFinish(dockspace_id);
		};

		if (ImGui::DockBuilderGetNode(dockspace_id) == NULL) {
			make_default_layout();
		}

		const auto dockspace_flags = 
			ImGuiDockNodeFlags_NoWindowMenuButton
			| ImGuiDockNodeFlags_NoCloseButton
			| ImGuiDockNodeFlags_PassthruCentralNode
			| ImGuiDockNodeFlags_NoDockingInCentralNode
		;

		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}

	perform_main_menu_bar(in);

	gui.inspector.perform({ 
		*this,
		in.necessary_images,
		in.ad_hoc_atlas,
		in.game_atlas
	});

	{
		auto& reveal_path = gui.inspector.reveal_in_explorer_once;

		if (!reveal_path.empty()) {
			in.window.reveal_in_explorer(reveal_path);

			reveal_path.clear();
		}
	}

	gui.layers.perform({ 
		*this, 
		project.layers,
		gui.filesystem.dragged_resource,
		in.ad_hoc_atlas,
		in.necessary_images
	});

	gui.filesystem.perform({ 
		*this,
		in.window,
		files.root,
		official_files_root,
		history,
		in.ad_hoc_atlas,
		in.necessary_images
	});

	gui.history.perform({ *this, history });

	gui.toolbar.perform({
			*this,
			in.window,
			in.ad_hoc_atlas,
			in.necessary_images
		});

	ImGui::End();

	if (autosave_popup) {
		if (autosave_popup->perform()) {
			autosave_popup = std::nullopt;
		}
	}
	else if (invalid_filenames_popup) {
		std::size_t can_be_renamed_num = 0;

		for (const auto& r : last_invalid_paths) {
			if (r.can_be_renamed()) {
				++can_be_renamed_num;
			}
		}

		if (can_be_renamed_num == 0) {
			if (invalid_filenames_popup->perform()) {
				invalid_filenames_popup = std::nullopt;
			}
		}
		else {
			if (const auto result = invalid_filenames_popup->perform({
				{ "Cancel" },
				{ 
					typesafe_sprintf("Rename (%x)", can_be_renamed_num), 
					rgba(200, 40, 0, 255),
					rgba(25, 20, 0, 255)
				}
			})) {
				invalid_filenames_popup = std::nullopt;

				if (const bool should_rename = result == 2) {
					/* Iterate in reverse so we first rename the descendants. */
					for (const auto& r : reverse(last_invalid_paths)) {
						if (r.can_be_renamed()) {
							const auto wrong = augs::make_windows_friendly(resolve_project_path(r.forbidden_path));
							const auto good = resolve_project_path(r.get_suggested_path());
							
							LOG("Renaming %x to %x.", wrong, good);

							try {
								LOG("Old exists: %x. New exists: %x.", augs::exists(wrong), augs::exists(good));

								if (augs::exists(wrong) && !augs::exists(good)) {
									std::filesystem::rename(wrong, good);
								}
							}
							catch (...) {

							}
						}
					}

					redirect_or_missing_popup = std::nullopt;
					on_window_activate();
				}
			}
		}
	}
	else if (redirect_or_missing_popup) {
		if (redirect_or_missing_popup->perform()) {
			redirect_or_missing_popup = std::nullopt;
		}
	}

	using namespace augs::imgui;

	{
		auto in = make_mover_input();

		if (const auto rot = mover.current_mover_rot_delta(in)) {
			text_tooltip("%x*\n(ESC or MMB to cancel)", *rot);
		}
		else if (const auto pos = mover.current_mover_pos_delta(in)) {
			auto final_pos = *pos;

			auto xs = "x";
			auto ys = "y";

			if (mover.show_absolute_mover_pos(in)) {
				final_pos += gui.filesystem.world_position_started_dragging;
				final_pos.discard_fract();
			}
			else {
				xs = "delta x";
				ys = "delta y";
			}

			text_tooltip("%x: %x\n%x: %x\n(ESC or MMB to cancel)", xs, final_pos.x, ys, final_pos.y);
		}
		else if (get_current_node_transforming_op() == node_mover_op::RESIZING) {
			text_tooltip("(ESC or MMB to cancel)");
		}
	}

	if (const auto node_id = get_hovered_node(); node_id.is_set()) {
		text_tooltip(get_name(node_id));
	}

	if (imgui_return_once) {
		const auto returned = *imgui_return_once;
		imgui_return_once = std::nullopt;

		return returned;
	}

	return custom_imgui_result::NONE;
}