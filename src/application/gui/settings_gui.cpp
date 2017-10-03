#include "augs/misc/imgui_control_wrappers.h"
#include "augs/misc/imgui_utils.h"
#include "augs/audio/audio_context.h"

#include "augs/window_framework/platform_utils.h"
#include "augs/window_framework/window.h"

#include "view/necessary_resources.h"

#include "application/config_lua_table.h"
#include "application/gui/settings_gui.h"

#include "generated/introspectors.h"

static void ShowHelpMarker(const char* const desc) {
	ImGui::TextDisabled("(?)");

	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(450.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

static bool operator==(const ImVec2 a, const ImVec2 b) {
	return a.x == b.x && a.y == b.y;
};

void configuration_subscribers::sync_back_into(config_lua_table& into) const {
	window.sync_back_into(into.window);
}

void configuration_subscribers::apply(const config_lua_table& new_config) const {
	DEBUG_DRAWING = new_config.debug_drawing;
	
	const auto screen_size = new_config.window.get_screen_size();

	fbos.apply(screen_size, new_config.drawing);
	window.apply(new_config.window);
	audio_context.apply(new_config.audio);
}

#define CONFIG_NVP(x) format_field_name(std::string(#x)) + "##" + std::to_string(field_id++), config.x

void settings_gui_state::perform(
	sol::state& lua,
	const augs::path_type& config_path_for_saving,
	config_lua_table& config,
	config_lua_table& last_saved_config
) {
	if (!show) {
		return;
	}

	{
		const auto screen_size = vec2(config.window.get_screen_size());
		const auto initial_settings_size = screen_size / 2;

		ImGui::SetNextWindowPos(screen_size / 2 - initial_settings_size / 2, ImGuiSetCond_FirstUseEver);
		ImGui::SetNextWindowSize(initial_settings_size, ImGuiSetCond_FirstUseEver);
	}

	ImGui::Begin("Settings", &show);

	ImGui::BeginChild("settings view", ImVec2(0, -(ImGui::GetItemsLineHeightWithSpacing() + 4)));
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.35f);

	int field_id = 0;
	using namespace augs::imgui;

	{
		auto tab_padding = ImGui::GetStyle().FramePadding;
		tab_padding.x *= 4;

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, tab_padding);
	}
	
	{
		static auto labels = []() {
			static augs::enum_array<std::string, settings_pane> label_strs;
			augs::enum_array<const char*, settings_pane> c_strs;

			augs::for_each_enum_except_bounds([&c_strs](const settings_pane s) {
				label_strs[s] = format_enum(s);
				c_strs[s] = label_strs[s].c_str();
			});

			c_strs[settings_pane::GUI_STYLES] = "GUI styles";

			return c_strs;
		}();

		auto index = static_cast<int>(active_pane);
		ImGui::TabLabels(labels.data(), labels.size(), index, nullptr);
		active_pane = static_cast<settings_pane>(index);
	}

	ImGui::PopStyleVar();

	ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyle().Colors[ImGuiCol_Button]);
	ImGui::Separator();
	ImGui::PopStyleColor();

	auto revert = make_revert_button_lambda(config, last_saved_config);

	auto revertable_checkbox = [&](auto l, auto& f, auto&&... args) {
		checkbox(l, f, std::forward<decltype(args)>(args)...);
		revert(f);
	};

	auto revertable_slider = [&](auto l, auto& f, auto&&... args) {
		slider(l, f, std::forward<decltype(args)>(args)...);
		revert(f);
	};

	auto revertable_drag_rect_bounded_vec2 = [&](auto l, auto& f, auto&&... args) {
		drag_rect_bounded_vec2(l, f, std::forward<decltype(args)>(args)...);
		revert(f);
	};

	switch (active_pane) {
		case settings_pane::WINDOW: {
			enum_combo("Launch on game's startup", config.launch_mode);

			revertable_checkbox("Fullscreen", config.window.fullscreen);
			if (!config.window.fullscreen) {
				auto indent = scoped_indent();
				
				{
					const auto disp = augs::get_display();

					revertable_drag_rect_bounded_vec2("Window position", config.window.position, 0.3f, vec2i{ 0, 0 }, disp.get_size(), "%.0f");
					revertable_drag_rect_bounded_vec2("Windowed size", config.window.size, 0.3f, vec2i{ 0, 0 }, disp.get_size(), "%.0f");
				}

				revertable_checkbox(CONFIG_NVP(window.border));
			}

			{
				text("Mouse input source");

				auto indent = scoped_indent();

				int e = config.window.raw_mouse_input ? 1 : 0;
				ImGui::RadioButton("Raw", &e, 1);
				
				if (ImGui::IsItemHovered()) {
					text_tooltip("Game draws its own cursor.\nWhen in GUI mode,\nforces the cursor inside the window.");
				}
#if TODO
				if (config.window.raw_mouse_input) {
					auto indent = scoped_indent();

					checkbox("But use system cursor for GUI", config.session.use_system_cursor_for_gui);
				}
#endif
				ImGui::RadioButton("System cursor", &e, 0);

				if (ImGui::IsItemHovered()) {
					text_tooltip("When in GUI mode,\nlets the cursor go outside the window.");
				}

				config.window.raw_mouse_input = e != 0;
			}

			input_text<100>(CONFIG_NVP(window.name)); revert(config.window.name);
			revertable_checkbox("Automatically hide settings in-game", config.session.automatically_hide_settings_ingame);

			break;
		}
		case settings_pane::GRAPHICS: {

			break;
		}
		case settings_pane::AUDIO: {
			revertable_slider("Music volume", config.audio_volume.music, 0.f, 1.f);
			revertable_slider("Sound effects volume", config.audio_volume.sound_effects, 0.f, 1.f);
			revertable_slider("GUI volume", config.audio_volume.gui, 0.f, 1.f);

			revertable_checkbox("Enable HRTF", config.audio.enable_hrtf);

			break;
		}
		case settings_pane::CONTROLS: {

			break;
		}
		case settings_pane::GAMEPLAY: {
			revertable_slider(CONFIG_NVP(camera.look_bound_expand), 0.2f, 2.f);
			
			revertable_checkbox(CONFIG_NVP(camera.enable_smoothing));

			if (config.camera.enable_smoothing) {
				auto indent = scoped_indent();

				revertable_slider(CONFIG_NVP(camera.smoothing.averages_per_sec), 0.f, 100.f); 
				revertable_slider(CONFIG_NVP(camera.smoothing.average_factor), 0.01f, 0.95f); 
			}

			revertable_checkbox("Draw weapon laser", config.drawing.draw_weapon_laser);
			revertable_checkbox("Draw crosshairs", config.drawing.draw_crosshairs);
			// checkbox("Draw gameplay GUI", config.drawing.draw_character_gui); revert(config.drawing.draw_character_gui);
			break;
		}
		case settings_pane::GUI_STYLES: {
			ImGuiStyle& style = config.gui_style;
			const ImGuiStyle& last_saved_style = last_saved_config.gui_style;

			if (ImGui::TreeNode("Rendering")) {
				revertable_checkbox("Anti-aliased lines", style.AntiAliasedLines);
				revertable_checkbox("Anti-aliased shapes", style.AntiAliasedShapes);
				ImGui::PushItemWidth(100);

				ImGui::DragFloat("Curve Tessellation Tolerance", &style.CurveTessellationTol, 0.02f, 0.10f, FLT_MAX, NULL, 2.0f);
				if (style.CurveTessellationTol < 0.0f) style.CurveTessellationTol = 0.10f;
				revert(style.CurveTessellationTol);

				ImGui::SliderFloat("Global Alpha", &style.Alpha, 0.20f, 1.0f, "%.2f"); revert(style.Alpha);// Not exposing zero here so user doesn't "lose" the UI (zero alpha clips all widgets). But application	code could have a toggle to switch between zero and non-zero.
				ImGui::PopItemWidth();
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Settings")) {
				ImGui::SliderFloat2("WindowPadding", (float*)&style.WindowPadding, 0.0f, 20.0f, "%.0f"); revert(style.WindowPadding);
				ImGui::SliderFloat("WindowRounding", &style.WindowRounding, 0.0f, 16.0f, "%.0f"); revert(style.WindowRounding);
				ImGui::SliderFloat("ChildWindowRounding", &style.ChildWindowRounding, 0.0f, 16.0f, "%.0f"); revert(style.ChildWindowRounding);
				ImGui::SliderFloat2("FramePadding", (float*)&style.FramePadding, 0.0f, 20.0f, "%.0f"); revert(style.FramePadding);
				ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 16.0f, "%.0f"); revert(style.FrameRounding);
				ImGui::SliderFloat2("ItemSpacing", (float*)&style.ItemSpacing, 0.0f, 20.0f, "%.0f"); revert(style.ItemSpacing);
				ImGui::SliderFloat2("ItemInnerSpacing", (float*)&style.ItemInnerSpacing, 0.0f, 20.0f, "%.0f"); revert(style.ItemInnerSpacing);
				ImGui::SliderFloat2("TouchExtraPadding", (float*)&style.TouchExtraPadding, 0.0f, 10.0f, "%.0f"); revert(style.TouchExtraPadding);
				ImGui::SliderFloat("IndentSpacing", &style.IndentSpacing, 0.0f, 30.0f, "%.0f"); revert(style.IndentSpacing);
				ImGui::SliderFloat("ScrollbarSize", &style.ScrollbarSize, 1.0f, 20.0f, "%.0f"); revert(style.ScrollbarSize);
				ImGui::SliderFloat("ScrollbarRounding", &style.ScrollbarRounding, 0.0f, 16.0f, "%.0f"); revert(style.ScrollbarRounding);
				ImGui::SliderFloat("GrabMinSize", &style.GrabMinSize, 1.0f, 20.0f, "%.0f"); revert(style.GrabMinSize);
				ImGui::SliderFloat("GrabRounding", &style.GrabRounding, 0.0f, 16.0f, "%.0f"); revert(style.GrabRounding);
				text("Alignment");
				ImGui::SliderFloat2("WindowTitleAlign", (float*)&style.WindowTitleAlign, 0.0f, 1.0f, "%.2f"); revert(style.WindowTitleAlign);
				ImGui::SliderFloat2("ButtonTextAlign", (float*)&style.ButtonTextAlign, 0.0f, 1.0f, "%.2f"); ImGui::SameLine(); ShowHelpMarker("Alignment applies when a button is larger than its text content.");	revert(style.ButtonTextAlign);
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Colors")) {
				text("Tip: Left-click on colored square to open color picker,\nRight-click to open edit options menu.");

				static ImGuiTextFilter filter;
				filter.Draw("Filter colors", 200);

				static ImGuiColorEditFlags alpha_flags = 0;
				ImGui::RadioButton("Opaque", &alpha_flags, 0); ImGui::SameLine();
				ImGui::RadioButton("Alpha", &alpha_flags, ImGuiColorEditFlags_AlphaPreview); ImGui::SameLine();
				ImGui::RadioButton("Both", &alpha_flags, ImGuiColorEditFlags_AlphaPreviewHalf);

				ImGui::BeginChild("#colors", ImVec2(0, 300), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
				ImGui::PushItemWidth(-160);
				for (int i = 0; i < ImGuiCol_COUNT; i++)
				{
					const char* name = ImGui::GetStyleColorName(i);
					if (!filter.PassFilter(name))
						continue;
					ImGui::PushID(i);
					ImGui::ColorEdit4(name, (float*)&style.Colors[i], ImGuiColorEditFlags_AlphaBar | alpha_flags);
					if (std::memcmp(&style.Colors[i], &last_saved_style.Colors[i], sizeof(ImVec4)) != 0) {
						ImGui::SameLine(); if (ImGui::Button("Revert")) style.Colors[i] = last_saved_style.Colors[i];
					}
					ImGui::PopID();
				}
				ImGui::PopItemWidth();
				ImGui::EndChild();

				ImGui::TreePop();
			}

			ImGui::GetStyle() = style;

			break;
		}
		case settings_pane::DEBUG: {
			revertable_checkbox("Show developer console", config.session.show_developer_console); 
			break;
		}
		default: {
			ensure(false && "Unknown settings pane type");
			break;
		}
	}

	ImGui::PopItemWidth();
	ImGui::EndChild();

	{
		const bool has_config_changed =
			!augs::equal_by_introspection(
				config,
				last_saved_config
			)
		;

		ImGui::BeginChild("save revert");

		ImGui::Separator();

		if (has_config_changed) {
			if (ImGui::Button("Save settings")) {
				augs::timer save_timer;
				last_saved_config = config;
				config.save(lua, config_path_for_saving);
				LOG("Saved new config in: %x ms", save_timer.get<std::chrono::milliseconds>());
			}

			ImGui::SameLine();

			if (ImGui::Button("Revert settings")) {
				config = last_saved_config;
				ImGui::GetStyle() = config.gui_style;
			}
		}

		ImGui::EndChild();
	}

	ImGui::End();
}

#undef CONFIG_NVP