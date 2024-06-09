#include "application/gui/social_sign_in_gui.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/window_framework/shell.h"

#include "application/setups/debugger/detail/maybe_different_colors.h"
#include "augs/misc/imgui/imgui_controls.h"

void sign_in_with_google();
void sign_in_with_discord();

social_sign_in_state::social_sign_in_state(const std::string& title)
    : standard_window_mixin<social_sign_in_state>(title) 
{}

bool social_sign_in_state::perform(social_sign_in_input in) {
	using namespace augs::imgui;
	
	if (!show) {
		return false;
	}

	center_next_window(ImGuiCond_Always);

	ImGui::SetNextWindowSize(ImVec2(450, in.prompted_once ? 260+60 : 335+60), ImGuiCond_Always);

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

	auto centered_text = [&](const std::string& str) {
		ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));

		auto colored_selectable = scoped_selectable_colors({
			rgba(255, 255, 255, 0),
			rgba(255, 255, 255, 0),
			rgba(255, 255, 255, 0)
		});

		ImGui::Selectable(str.c_str(), false);

		ImGui::PopStyleVar();
	};

	auto reason = last_reason;

	if (reason.empty()) {
		reason = "compete on Leaderboards";
	}

	centered_text(typesafe_sprintf("Sign in to %x:", reason));

	ImGui::Separator();

	ImGui::PopFont();
	text(" ");

	auto& imgs = in.necessary_images;

	auto login_option = [&](const auto icon, const auto label, const float sz_mult = 1.0f, const vec2 pad = vec2(0.5, 0.2), float alpha=1.0f, std::string tooltip="") {
		const bool even = int(icon) % 2 == 0;

		auto pd = scoped_style_var(ImGuiStyleVar_ItemSpacing, ImVec2(ItemSpacing.x, 24));

		const auto result = selectable_with_icon(
			imgs[icon],
			label,
			sz_mult,
			pad,
			white,
			{
				rgba(255, 255, 255, alpha*(even ? 15 : 15)),
				rgba(255, 255, 255, alpha*30),
				rgba(255, 255, 255, alpha*60)
			},
			0.0f,
			true,
			[&tooltip]() { if (!tooltip.empty()) { tooltip_on_hover(tooltip); } }
		);

		return result;
	};

	using N = assets::necessary_image_id;

#if 0
	if (login_option(N::SOCIAL_GOOGLE, "Sign in with Google")) {
		::sign_in_with_google();
	}
#endif

	{
		auto sc = scoped_text_color(rgba(255, 255, 255, 230));

		if (login_option(N::SOCIAL_STEAM, "(opt.) Connect Discord with Steam", 1.0f, vec2(0.5, 0.2), 0.6f, "(Optional)\nMatches played with Discord account\nwill count towards your Steam account.\n\nYou may later disconnect the two accounts\nwithout losing any progress.")) {
			augs::open_url("https://hypersomnia.xyz/profile");
		}
	}

	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

	if (login_option(N::SOCIAL_DISCORD, "Sign in with Discord")) {
#if PLATFORM_WEB
		::sign_in_with_discord();
#endif
	}

	ImGui::PopFont();

	text(" ");
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

	ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));

	if (!in.prompted_once) {
		centered_text("...or play as Guest:");

		{
			auto sc = scoped_style_color(ImGuiCol_FrameBg, rgba(255,255,255,10));

			ImGui::SetNextItemWidth(-0.0001f);
			auto comfier_frame_padding = scoped_style_var(ImGuiStyleVar_FramePadding, final_frame_padding);

			base::acquire_keyboard_once();
			input_text<max_nickname_length_v>("##NicknameChooser", guest_nickname);
		}
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

	bool ok = false;

	if (in.prompted_once) {
		ok = ImGui::Selectable("Cancel##ConfirmCancel", true);
	}
	else {
		auto scope = maybe_disabled_cols({}, guest_nickname.size() < min_nickname_length_v);

		ok = ImGui::Selectable("OK##ConfirmGuest", true);
	}

	ImGui::PopStyleVar();

	ImGui::PopFont();

	return ok;
}
