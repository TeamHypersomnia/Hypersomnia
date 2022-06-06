#include "game/cosmos/logic_step.h"
#include "game/organization/all_messages_includes.h"

#include "view/viewables/viewables_loading_type.h"

#include "application/config_lua_table.h"
#include "application/setups/editor/editor_setup.h"

#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
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
		if (auto menu = scoped_menu("Project")) {

		}

		if (auto menu = scoped_menu("Edit")) {

		}

		if (auto menu = scoped_menu("Window")) {
			auto do_window_entry = [&](auto& win, const auto shortcut) {
				const auto s = std::string("ALT+") + shortcut;
				if (item_if_tabs(win.get_title().c_str(), s.c_str())) {
					win.open();
				}
			};

			do_window_entry(gui.hierarchy, "R");
			do_window_entry(gui.project_files, "P");
			do_window_entry(gui.inspector, "I");
		}
	}
}

custom_imgui_result editor_setup::perform_custom_imgui(const perform_custom_imgui_input in) {
	(void)in;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

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

		auto open_default_windows = [&]() {
			gui.inspector.open();
			gui.hierarchy.open();
			gui.project_files.open();
		};

		auto make_default_layout = [&]() {
			ImGui::DockBuilderRemoveNode(dockspace_id); // Clear out existing layout
			ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace); // Add empty node
			ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetContentRegionAvail());

			ImGuiID dock_main_id = dockspace_id; // This variable will track the document node, however we are not using it here as we aren't docking anything into it.

			ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.20f, NULL, &dock_main_id);
			ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.20f, NULL, &dock_main_id);
			ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, NULL, &dock_main_id);

			ImGui::DockBuilderDockWindow(gui.project_files.get_title().c_str(), dock_id_bottom);
			ImGui::DockBuilderDockWindow(gui.hierarchy.get_title().c_str(), dock_id_left);
			ImGui::DockBuilderDockWindow(gui.inspector.get_title().c_str(), dock_id_right);
			ImGui::DockBuilderFinish(dockspace_id);
		};

		if (ImGui::DockBuilderGetNode(dockspace_id) == NULL) {
			open_default_windows();
			make_default_layout();
		}

		const auto dockspace_flags = 
			ImGuiDockNodeFlags_NoWindowMenuButton
			| ImGuiDockNodeFlags_NoCloseButton
			| ImGuiDockNodeFlags_PassthruCentralNode
		;

		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}

	perform_main_menu_bar(in);

	gui.inspector.perform({});
	gui.hierarchy.perform({});
	gui.project_files.perform({});

	ImGui::End();

	return custom_imgui_result::NONE;
}