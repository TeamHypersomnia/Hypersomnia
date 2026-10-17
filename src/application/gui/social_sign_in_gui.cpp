#include "application/gui/social_sign_in_gui.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/window_framework/shell.h"

#include "augs/misc/imgui/maybe_different.h"
#include "augs/misc/imgui/imgui_controls.h"

void sign_in_with_crazygames();

social_sign_in_state::social_sign_in_state(const std::string& title)
    : standard_window_mixin<social_sign_in_state>(title) 
{}

bool social_sign_in_state::perform(social_sign_in_input in) {
	using namespace augs::imgui;
	
	if (!show) {
		return false;
	}

	const float ratio = ImGui::GetTextLineHeight() / 22.0f;

	center_next_window(ImGuiCond_Always);

	{
		auto h = in.prompted_once ? 260+60 : 335+60;

		if (in.is_crazygames) {
			h -= 60;
		}
		else {
			/*
				Outside CrazyGames there is nothing to sign in to - the account is the key in the browser.
			*/

			h -= 120;
		}

		ImGui::SetNextWindowSize(ImVec2(450 * ratio, h * ratio), ImGuiCond_Always);
	}

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
		return ImVec2(WinPadding.x * 2.8f * ratio, WinPadding.y * 2.8f * ratio);
	}();

	const auto final_frame_padding = [&]() {
		return ImVec2(FramePadding.x * 2.f * ratio, FramePadding.y * 2.f * ratio);
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

	if (in.is_crazygames) {
		centered_text(typesafe_sprintf("Sign in to %x:", reason));
	}
	else {
		centered_text("Choose your nickname:");
	}

	ImGui::Separator();

	ImGui::PopFont();
	text(" ");

	auto& imgs = in.necessary_images;

	auto login_option = [&](const auto icon, const auto label, const float sz_mult = 1.0f, const vec2 pad = vec2(0.5, 0.2), float alpha=1.0f, std::string tooltip="") {
		const bool even = int(icon) % 2 == 0;

		auto pd = scoped_style_var(ImGuiStyleVar_ItemSpacing, ImVec2(ItemSpacing.x * ratio, 24 * ratio));

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

	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

	if (in.is_crazygames) {
		if (login_option(N::SOCIAL_CRAZYGAMES, "Sign in with CrazyGames")) {
#if PLATFORM_WEB
			::sign_in_with_crazygames();
#endif
		}
	}

	ImGui::PopFont();

	text(" ");
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

	ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));

	if (!in.prompted_once) {
		if (in.is_crazygames) {
			centered_text("...or play as Guest:");
		}

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

	auto pd = scoped_style_var(ImGuiStyleVar_ItemSpacing, ImVec2(ItemSpacing.x * ratio, 16 * ratio));

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
