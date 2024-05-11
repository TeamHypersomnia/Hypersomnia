#include "application/gui/social_sign_in_gui.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/window_framework/shell.h"

#include "augs/misc/imgui/imgui_controls.h"

social_sign_in_state::social_sign_in_state(const std::string& title)
    : standard_window_mixin<social_sign_in_state>(title) 
{}

bool social_sign_in_state::perform(social_sign_in_input in) {
	using namespace augs::imgui;
	
	if (!show) {
		return false;
	}

	center_next_window(ImGuiCond_Always);

	ImGui::SetNextWindowSize(ImVec2(450, 385), ImGuiCond_Always);

    const auto flags = 
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoScrollbar | 
		ImGuiWindowFlags_NoScrollWithMouse
	;

	const auto WinPadding = ImGui::GetStyle().WindowPadding;
	const auto FramePadding = ImGui::GetStyle().FramePadding;
	const auto ItemSpacing = ImGui::GetStyle().ItemSpacing;

	const auto final_padding = [&]() {
		return ImVec2(WinPadding.x * 2.8f, WinPadding.y * 2.8f);
	}();

	const auto final_frame_padding = [&]() {
		return ImVec2(FramePadding.x * 2.f, FramePadding.y * 2.f);
	}();

	auto comfier_padding = scoped_style_var(ImGuiStyleVar_WindowPadding, final_padding);

	auto imgui_window = augs::imgui::scoped_modal_popup("SignInModal", nullptr, flags);

	if (!imgui_window) {
		return false;
	}

	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

	auto centered_text = [&](auto str) {
		ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));

		auto colored_selectable = scoped_selectable_colors({
			rgba(255, 255, 255, 0),
			rgba(255, 255, 255, 0),
			rgba(255, 255, 255, 0)
		});

		ImGui::Selectable(str, false);

		ImGui::PopStyleVar();
	};

	centered_text("Sign in to compete on Leaderboards:");

	ImGui::Separator();

	ImGui::PopFont();
	text(" ");
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

	auto& imgs = in.necessary_images;

	auto login_option = [&](const auto icon, const auto label) {
		const bool even = int(icon) % 2 == 0;

		auto pd = scoped_style_var(ImGuiStyleVar_ItemSpacing, ImVec2(ItemSpacing.x, 24));

		const auto result = selectable_with_icon(
			imgs[icon],
			label,
			1.0f,
			vec2(0.5, 0.2),
			white,
			{
				rgba(255, 255, 255, even ? 15 : 15),
				rgba(255, 255, 255, 30),
				rgba(255, 255, 255, 60)
			}
		);

		return result;
	};

	using N = assets::necessary_image_id;

	if (login_option(N::SOCIAL_GOOGLE, "Sign in with Google")) {

	}

	if (login_option(N::SOCIAL_DISCORD, "Sign in with Discord")) {

	}

	ImGui::PopFont();
	text(" ");
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

	ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));

	centered_text("...or play as Guest:");

	{
		auto sc = scoped_style_color(ImGuiCol_FrameBg, rgba(255,255,255,10));

		ImGui::SetNextItemWidth(-0.0001f);
		auto comfier_frame_padding = scoped_style_var(ImGuiStyleVar_FramePadding, final_frame_padding);

		base::acquire_keyboard_once();
		input_text("##NicknameChooser", guest_nickname);
	}

	ImGui::Separator();

	auto colored_selectable = scoped_selectable_colors({
		rgba(255, 255, 255, 20),
		rgba(255, 255, 255, 30),
		rgba(255, 255, 255, 60)
	});

	ImGui::PopFont();
	text(" ");
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

	auto pd = scoped_style_var(ImGuiStyleVar_ItemSpacing, ImVec2(ItemSpacing.x, 16));
	const bool ok = ImGui::Selectable("OK##ConfirmGuest", true);

	ImGui::PopStyleVar();

	ImGui::PopFont();

	return ok;
}
