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

void editor_setup::perform_main_menu_bar(const perform_custom_imgui_input in) {
	using namespace augs::imgui;

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
		if (auto menu = scoped_menu(project.meta.name.c_str())) {

		}

		if (auto menu = scoped_menu("Edit")) {

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
		}
	}
}

custom_imgui_result editor_setup::perform_custom_imgui(const perform_custom_imgui_input in) {
	(void)in;

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

			ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, NULL, &dock_main_id);
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
		gui.filesystem.showing_official() ? official_files_root : files.root,

		in.ad_hoc_atlas,
		in.necessary_images
	});

	gui.history.perform({ *this, history });

	gui.toolbar.perform({
		*this,
		in.window,
		in.necessary_images
	});

	ImGui::End();

	if (ok_only_popup) {
		if (ok_only_popup->perform()) {
			ok_only_popup = std::nullopt;
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

	return custom_imgui_result::NONE;
}