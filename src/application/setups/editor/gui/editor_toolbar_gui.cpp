#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_controls.h"
#include "augs/misc/imgui/imgui_game_image.h"

#include "application/setups/editor/editor_setup.hpp"
#include "application/setups/editor/gui/editor_toolbar_gui.h"
#include "application/setups/editor/project/editor_project.h"
#include "application/setups/editor/editor_setup.h"
#include "application/setups/editor/gui/widgets/icon_button.h"
#include "application/setups/editor/gui/id_widget_handler.h"

#include "3rdparty/imgui/imgui_internal.h"
#include "augs/window_framework/window.h"

template <class T>
void DockingToolbar(const char* name, ImGuiAxis* p_toolbar_axis, bool& is_docked, T icons_callback)
{

    const ImVec2 icon_size(32, 32);
    // [Option] Automatically update axis based on parent split (inside of doing it via right-click on the toolbar)
    // Pros:
    // - Less user intervention.
    // - Avoid for need for saving the toolbar direction, since it's automatic.
    // Cons: 
    // - This is currently leading to some glitches.
    // - Some docking setup won't return the axis the user would expect.
    const bool TOOLBAR_AUTO_DIRECTION_WHEN_DOCKED = true;

    // ImGuiAxis_X = horizontal toolbar
    // ImGuiAxis_Y = vertical toolbar
    ImGuiAxis toolbar_axis = *p_toolbar_axis;

    // 1. We request auto-sizing on one axis
    // Note however this will only affect the toolbar when NOT docked.
	ImVec2 requested_size = (toolbar_axis == ImGuiAxis_X) ? ImVec2(-1.0f, icon_size.y + ImGui::GetStyle().WindowPadding.y*2) : ImVec2(icon_size.x + ImGui::GetStyle().WindowPadding.x * 2, -1.0f);
    ImGui::SetNextWindowSize(requested_size);

    // 2. Specific docking options for toolbars.
    // Currently they add some constraint we ideally wouldn't want, but this is simplifying our first implementation
    ImGuiWindowClass window_class;
    window_class.DockingAllowUnclassed = true;
	//window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoCloseButton;
    window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_HiddenTabBar; // ImGuiDockNodeFlags_NoTabBar // FIXME: Will need a working Undock widget for _NoTabBar to work
    window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoDockingSplitMe;
    window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoDockingOverMe;
    window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoDockingOverOther;
    if (toolbar_axis == ImGuiAxis_X)
        window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoResizeY;
    else
        window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoResizeX;
    ImGui::SetNextWindowClass(&window_class);

    // 3. Begin into the window
    ImGui::Begin(name, NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    // 4. Overwrite node size
    ImGuiDockNode* node = ImGui::GetWindowDockNode();
    if (node != NULL)
    {
        // Overwrite size of the node
        ImGuiStyle& style = ImGui::GetStyle();
        const ImGuiAxis toolbar_axis_perp = (ImGuiAxis)(toolbar_axis ^ 1);

		if (toolbar_axis_perp == 0 || toolbar_axis_perp == 1) {
			const float TOOLBAR_SIZE_WHEN_DOCKED = style.WindowPadding[toolbar_axis_perp] * 2.0f + icon_size[toolbar_axis_perp];
			node->WantLockSizeOnce = true;
			node->Size[toolbar_axis_perp] = node->SizeRef[toolbar_axis_perp] = TOOLBAR_SIZE_WHEN_DOCKED;

			if (TOOLBAR_AUTO_DIRECTION_WHEN_DOCKED)
			if (node->ParentNode && node->ParentNode->SplitAxis != ImGuiAxis_None)
			toolbar_axis = (ImGuiAxis)(node->ParentNode->SplitAxis ^ 1);
		}
    }
    
    // 5. Dummy populate tab bar
	//ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
	//ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
    //UndockWidget(icon_size, toolbar_axis);
	icons_callback(toolbar_axis);

	//ImGui::PopStyleVar(1);

	is_docked = ImGui::IsWindowDocked();

    // 6. Context-menu to change axis
	if (!is_docked || !TOOLBAR_AUTO_DIRECTION_WHEN_DOCKED)
    {
        if (ImGui::BeginPopupContextWindow())
        {
            ImGui::TextUnformatted(name);
            ImGui::Separator();
            if (ImGui::MenuItem("Horizontal", "", (toolbar_axis == ImGuiAxis_X)))
                toolbar_axis = ImGuiAxis_X;
            if (ImGui::MenuItem("Vertical", "", (toolbar_axis == ImGuiAxis_Y)))
                toolbar_axis = ImGuiAxis_Y;
            ImGui::EndPopup();
        }
    }

    ImGui::End();

    // Output user stored data
    *p_toolbar_axis = toolbar_axis;
}

void editor_toolbar_gui::perform(const editor_toolbar_input in) {
	using namespace augs::imgui;
	using namespace editor_widgets;

	if (!show) {
		return;
	}

	//const auto scoped_style = scoped_style_var(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

	const auto WinPadding = ImGui::GetStyle().WindowPadding;
	const auto icon_size = 32.0f;
	const auto min_window_size = ImVec2(icon_size + WinPadding.x * 2, icon_size + WinPadding.y * 2);
	(void)min_window_size;

	const auto final_padding = [&]() {
		if (is_docked) {
			if (toolbar_axis == ImGuiAxis_X) {
				return ImVec2(WinPadding.x * 2.0f, WinPadding.y);
			}
			else {
				return ImVec2(WinPadding.x * 2.2f, WinPadding.y * 2.0f);
			}
		}

		return ImVec2(WinPadding.x * 2.8f, WinPadding.y * 2.8f);
	}();

	auto comfier_padding = scoped_style_var(ImGuiStyleVar_WindowPadding, final_padding);

	const bool node_or_layer_inspected = 
		in.setup.inspects_any<editor_node_id>()
		|| in.setup.inspects_any<editor_layer_id>()
	;
	(void)node_or_layer_inspected;

	int current_icon_id = 0;

	const bool any_node_selected = in.setup.inspects_any<editor_node_id>();

	using ID = assets::necessary_image_id;

	auto img = [&](const auto id) {
		return in.necessary_images.at(id);
	};

	const std::array<rgba, 3> icon_bg_cols = {
		rgba(0, 0, 0, 0),
		rgba(35, 60, 90, 255),
		rgba(35+10, 60+10, 90+10, 255)
	};

	auto do_icon = [this, icon_bg_cols, &current_icon_id, img, icon_size](auto img_id, auto tooltip, bool enabled = true, bool currently_active = false, rgba tint = white) {
		auto scope = scoped_id(current_icon_id++);

		auto cols = icon_bg_cols;

		if (currently_active) {
			cols[0] = cols[1];
			tint *= green;
		}

		auto result = icon_button(
			"##Transform",
			img(img_id),
			[](){},
			tooltip,
			enabled,
			tint,
			cols,
			icon_size
		);

		if (toolbar_axis == ImGuiAxis_X) {
			ImGui::SameLine();
		}

		return result;
	};

	auto category_separator = [&]() {
		if (toolbar_axis == ImGuiAxis_X) {
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine();
		}
		else {
			ImGui::Separator();
		}
	};

	auto icons_callback = [&](const ImGuiAxis) {
		if (!is_docked) {
			auto no_space = scoped_style_var(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
			ImGui::Separator();

			//auto no_space = scoped_style_var(ImGuiStyleVar_ItemSpacing, ImVec2(5.0f, 5.0f));
			auto pos = ImGui::GetCursorPos();
			pos.y += 5;
			ImGui::SetCursorPos(pos);
		}

		//auto no_space = scoped_style_var(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

		const auto op = in.setup.get_current_node_transforming_op();

		auto warp_cursor = [&]() {
			in.setup.warp_cursor_to_center(in.window);
		};

		if (do_icon(ID::EDITOR_TOOL_MOVE, "Move selection (T)", any_node_selected, op == node_mover_op::TRANSLATING)) {
			warp_cursor();
			in.setup.start_moving_selection();
		}

		if (do_icon(ID::EDITOR_TOOL_ROTATE_ARBITRARY, "Rotate selection (R)\n(W to reset rotations to 0)", any_node_selected, op == node_mover_op::ROTATING)) {
			in.setup.start_rotating_selection();
		}

		const auto resize_desc = "Resize selection\n(place cursor near the chosen edge and press E)\n(Ctrl+E to pick two edges simultaneously)";

		const bool resizing_active = ImGui::IsPopupOpen("Resize Options");

		if (do_icon(ID::EDITOR_TOOL_RESIZE, resize_desc, in.setup.can_resize_selected_nodes(), op == node_mover_op::RESIZING || resizing_active)) {
			ImGui::OpenPopup("Resize Options");
		}

		if (ImGui::BeginPopup("Resize Options")) {
			auto padding = vec2(0.5f, 0.0f);
			const bool pad_from_left = false;

			using ae = resize_nodes_command::active_edges;
			auto do_resize_opt = [&](
				const auto label,
				const bool two,
				const auto edges,
				const auto edge_icon_angle
			) {
				if (selectable_with_icon(img(two ? ID::EDITOR_TOOL_RESIZE_TWO_EDGES : ID::EDITOR_TOOL_RESIZE_EDGE), label, 2.0f, padding, white, icon_bg_cols, edge_icon_angle, pad_from_left)) {
					in.setup.start_resizing_selection(two, edges);
				}
			};

			do_resize_opt("Drag top edge", false, ae(true, false, false, false), -90);
			do_resize_opt("Drag left edge", false, ae(false, true, false, false), -180);
			do_resize_opt("Drag right edge", false, ae(false, false, true, false), 0);
			do_resize_opt("Drag bottom edge", false, ae(false, false, false, true), 90);

			ImGui::Separator();

			do_resize_opt("Drag top-left edges", true, ae(true, true, false, false), -90);
			do_resize_opt("Drag top-right edges", true, ae(true, false, true, false), 0);
			do_resize_opt("Drag bottom-right edges", true, ae(false, false, true, true), 90);
			do_resize_opt("Drag bottom-left edges", true, ae(false, true, false, true), 180);

			ImGui::EndPopup();
		}

		if (do_icon(ID::EDITOR_TOOL_ROTATE_CCW, "Rotate selection by -90 degrees (Shift+R)", any_node_selected)) {
			in.setup.rotate_selection_once_by(-90);
		}

		if (do_icon(ID::EDITOR_TOOL_ROTATE_CW, "Rotate selection by 90 degrees (Ctrl+R)", any_node_selected)) {
			in.setup.rotate_selection_once_by(90);
		}


		if (do_icon(ID::EDITOR_TOOL_FLIP_HORIZONTALLY, "Flip selection horizontally (Shift+H)", any_node_selected)) {
			in.setup.flip_selection_horizontally();
		}

		if (do_icon(ID::EDITOR_TOOL_FLIP_VERTICALLY, "Flip selection vertically (Shift+V)", any_node_selected)) {
			in.setup.flip_selection_vertically();
		}

		category_separator();

		if (do_icon(ID::EDITOR_ICON_CLONE, "Clone selection (C)", node_or_layer_inspected)) {
			if (in.setup.should_warp_cursor_before_cloning()) {
				warp_cursor();
			}

			in.setup.clone_selection();
		}

		const bool mirroring_active = ImGui::IsPopupOpen("Mirror Options");

		if (do_icon(ID::EDITOR_TOOL_MIRROR, "Clone selection with mirroring (Ctrl+Arrow)", node_or_layer_inspected, mirroring_active)) {
			ImGui::OpenPopup("Mirror Options");
		}

		if (ImGui::BeginPopup("Mirror Options")) {
			auto padding = vec2(0.5f, 0.0f);
			const bool pad_from_left = false;

			auto do_mirror_opt = [&](const auto label, const auto dir, const auto arrow_icon_angle) {
				if (selectable_with_icon(img(ID::EDITOR_TOOL_PLAIN_ARROW), label, 2.0f, padding, white, icon_bg_cols, arrow_icon_angle, pad_from_left)) {
					in.setup.mirror_selection(dir);
				}
			};

			do_mirror_opt("Mirror up (Ctrl+Up)", vec2i(0, -1), -90);
			do_mirror_opt("Mirror left (Ctrl+Left)", vec2i(-1, 0), -180);
			do_mirror_opt("Mirror right (Ctrl+Right)", vec2i(1, 0), 0);
			do_mirror_opt("Mirror down (Ctrl+Down)", vec2i(0, 1), 90);

			ImGui::EndPopup();
		}

		category_separator();

		const bool grid_enabled = in.setup.is_grid_enabled();
		const bool snapping_enabled = in.setup.is_snapping_enabled();

		const auto grid_icon = grid_enabled ? ID::EDITOR_TOOL_GRID_ENABLED : ID::EDITOR_TOOL_GRID_DISABLED;
		const auto snap_icon = snapping_enabled ? ID::EDITOR_TOOL_SNAPPING_ENABLED : ID::EDITOR_TOOL_SNAPPING_DISABLED;
		const auto grid_desc = grid_enabled ? "Showing grid (G to toggle)" : "Hiding grid (G to toggle)";
		const auto snap_desc = snapping_enabled ? "Snapping to grid enabled (S to toggle)" : "Snapping to grid disabled (S to toggle)";

		const auto current_units = typesafe_sprintf("Current grid size: %x pixels", in.setup.get_current_grid_size());

		if (do_icon(grid_icon, grid_desc)) {
			in.setup.toggle_grid();
		}

		if (do_icon(ID::EDITOR_TOOL_GRID_SPARSER, "Sparser grid ([)\n" + current_units, grid_enabled)) {
			in.setup.sparser_grid();
		}

		if (do_icon(ID::EDITOR_TOOL_GRID_DENSER, "Denser grid (])\n" + current_units, grid_enabled)) {
			in.setup.denser_grid();
		}

		if (do_icon(snap_icon, snap_desc)) {
			in.setup.toggle_snapping();
		}

		category_separator();

		const auto zoom = in.setup.get_camera_eye().zoom;
		const bool nonzero_zoom = zoom != 1.0f;

		const auto current_zoom = typesafe_sprintf("\nCurrent zoom: %2f%", zoom * 100.0f);
		const bool is_centered = in.setup.is_view_centered_at_selection();

		if (do_icon(ID::EDITOR_TOOL_CENTER_VIEW, "Focus view on selection (F)\n(or double-click a node on scene)", !is_centered)) {
			in.setup.center_view_at_selection();
		}

		if (do_icon(ID::EDITOR_TOOL_RESET_ZOOM, "Reset zoom (Z or =)\n(-, + or mouse scroll to change zoom)" + current_zoom, nonzero_zoom)) {
			in.setup.reset_zoom();
		}

		float zi = (zoom * 100);

		ImGui::SetNextItemWidth(ImGui::CalcTextSize("9999% ").x);

		auto fmt = "%.0f%%";

		{
			auto current_padding = ImGui::GetStyle().FramePadding;

			if (toolbar_axis == ImGuiAxis_Y) {

				if (is_docked) {
					if (zi >= 100) {
						current_padding.x = 0.0f;
					}
				}

				auto pos = ImGui::GetCursorPos();
				pos.x -= ImGui::GetStyle().WindowPadding.x/2;
				ImGui::SetCursorPos(pos);
			}

			const auto scoped_style = scoped_style_var(ImGuiStyleVar_FramePadding, current_padding);

			if (ImGui::InputFloat("##ZoomInput", &zi, 0.0f, 0.0f, fmt)) {
				in.setup.set_zoom(std::clamp(float(zi) / 100, 0.01f, 10.0f));
			}
		}

		if (toolbar_axis == ImGuiAxis_X) {
			ImGui::SameLine();
		}

		category_separator();

		if (do_icon(ID::EDITOR_TOOL_PLAYTEST, "Playtest (Space)")) {
			in.setup.start_playtesting();
		}

		if (do_icon(ID::EDITOR_TOOL_HOST_SERVER, "Playtest online (Ctrl+P)\n\nThis playtesting session will be visible in the server browser.\nYour friends will be able to join!")) {

			in.setup.request_online_playtesting();
		}

		{
			auto id_handler = id_widget_handler { in.setup, editor_icon_info_in(in), false };

			auto mode = in.setup.get_project().playtesting.mode;

			std::string result;

			if (const auto res = in.setup.find_resource(mode)) {
				const auto name = res->get_display_name() + "  ";

				if (toolbar_axis == ImGuiAxis_Y) {
					auto pos = ImGui::GetCursorPos();
					pos.x -= ImGui::GetStyle().WindowPadding.x/2;
					ImGui::SetCursorPos(pos);
				}

				ImGui::SetNextItemWidth(ImGui::CalcTextSize(name.c_str()).x + ImGui::GetFrameHeight());

				if (id_handler.handle(result, "##ModeChooserWidget", mode, false)) {
					auto new_playtesting = in.setup.get_project().playtesting;
					new_playtesting.mode = mode;

					edit_project_settings_command cmd;
					cmd.do_inspector_at_all = false;
					cmd.after = std::move(new_playtesting);
					cmd.built_description = typesafe_sprintf("Set playtested mode to %x", in.setup.get_name(in.setup.get_project().playtesting.mode));

					in.setup.post_new_command(std::move(cmd));
				}

				
				if (ImGui::IsItemHovered()) {
					text_tooltip("Mode to launch after pressing 'Playtest' buttons.\nApplies to both offline and online playtesting.\n\nTo choose the default mode launched by an actual server,\ngo to project settings (CTRL+U).");

				}
				if (toolbar_axis == ImGuiAxis_X) {
					ImGui::SameLine();
				}
			}
		}

		category_separator();

		auto tooltip = typesafe_sprintf("Upload the arena to:\n%x", in.setup.get_settings().upload_url);

		if (!in.setup.upload_icon_visible()) {
			tooltip += "\n\nYou need to provide an API key.\nGo to Settings -> Editor -> Uploading.";
		}
		else {
			tooltip += "\n\n(CTRL+SHIFT+U)";
		}

		if (do_icon(ID::UPLOAD_ICON, tooltip, in.setup.upload_icon_visible(), in.setup.upload_in_progress())) {
			in.setup.upload_to_arena_server();
		}
	};

	DockingToolbar(get_title().c_str(), &toolbar_axis, is_docked, icons_callback);
}
