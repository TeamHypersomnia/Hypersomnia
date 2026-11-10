#include <algorithm>
#include <cmath>
#include "augs/log.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_game_image.h"
#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/async_get.h"
#include "augs/misc/date_time.h"
#include "augs/readwrite/to_bytes.h"
#include "augs/string/parse_url.h"
#include "augs/string/typesafe_sprintf.h"
#include "augs/window_framework/shell.h"
#include "application/gui/dev_journal_gui.h"
#include "application/gui/leaderboards_gui.h"

bool should_show_dev_journal(const std::uint64_t seen_premiere_unix_utc) {
#if !IS_PRODUCTION_BUILD
	(void)seen_premiere_unix_utc;
	return false;
#else
	const auto& journal = latest_dev_journal_v;
	const auto now = static_cast<std::uint64_t>(augs::date_time::secs_since_epoch());

	const bool announced = journal.youtube_url[0] != '\0';
	const bool premiered = journal.premiere_unix_utc <= now && now < journal.shown_until_unix_utc;
	const bool seen = seen_premiere_unix_utc >= journal.premiere_unix_utc;

	return announced && premiered && !seen;
#endif
}

void dev_journal_window_state::open_fetching_thumbnail(const std::string& website_url, const bool green_blood_thumbnail) {
	open();

	const auto location_new_for_every_premiere = typesafe_sprintf(
		"%x?premiere=%x",
		green_blood_thumbnail ? dev_journal_green_blood_thumbnail_location_v : dev_journal_thumbnail_location_v,
		latest_dev_journal_v.premiere_unix_utc
	);

	if (const auto parsed = parsed_url(website_url); parsed.valid()) {
		thumbnail_response = augs::async_get(parsed.get_base_url(), location_new_for_every_premiere);
	}
}

static std::optional<augs::image> decode_dev_journal_thumbnail(const augs::http_response& response) {
	if (response.status != 200) {
		LOG("No thumbnail of the dev journal (HTTP %x).", response.status);
		return std::nullopt;
	}

	try {
		auto image = augs::image();
		image.from_bytes_stbi(augs::string_to_bytes(response.body), "dev journal thumbnail");
		return image;
	}
	catch (const std::exception& err) {
		LOG("Could not read the thumbnail of the dev journal: %x", err.what());
		return std::nullopt;
	}
}

static vec2 fit_keeping_aspect(const vec2 size, const vec2 bounds) {
	const auto scale = std::min(bounds.x / size.x, bounds.y / size.y);
	return (size * scale).discard_fract();
}

static bool perform_dev_journal_thumbnail(const vec2u thumbnail_size, const vec2 area_lt, const vec2 area_size) {
	using namespace augs::imgui;

	const auto size = ::fit_keeping_aspect(vec2(thumbnail_size), area_size);

	if (size.x < 1.0f || size.y < 1.0f) {
		return false;
	}

	const auto lt = area_lt + ((area_size - size) / 2).discard_fract();

	ImGui::SetCursorPos(static_cast<ImVec2>(lt));
	const bool clicked = ImGui::InvisibleButton("##dev_journal_thumbnail", static_cast<ImVec2>(size));
	const bool hovered = ImGui::IsItemHovered();

	if (hovered) {
		ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
	}

	auto whole_texture = augs::atlas_entry();
	whole_texture.atlas_space = xywh(0, 0, 1, 1);

	const auto tint = hovered ? white : rgba(200, 200, 200, 255);

	ImGui::SetCursorPos(static_cast<ImVec2>(lt));
	game_image(whole_texture, size, tint, vec2::zero, augs::imgui_atlas_type::DEV_JOURNAL_THUMBNAIL);

	if (hovered) {
		const auto screen_lt = vec2(ImGui::GetCursorScreenPos());
		const auto frame_color = ImGui::GetColorU32(leaderboards_tint);
		const auto frame_thickness = 2.0f;

		ImGui::GetWindowDrawList()->AddRect(
			static_cast<ImVec2>(screen_lt),
			static_cast<ImVec2>(screen_lt + size),
			frame_color,
			0.0f,
			0,
			frame_thickness
		);
	}

	return clicked;
}

static void center_larger_text(const char* const text, const vec2 area_lt, const vec2 area_size) {
	const auto larger_font = ImGui::GetIO().Fonts->Fonts[1];

	ImGui::PushFont(larger_font);

	const auto text_size = vec2(ImGui::CalcTextSize(text));

	ImGui::SetCursorPos(static_cast<ImVec2>(area_lt + ((area_size - text_size) / 2).discard_fract()));
	augs::imgui::text_color(text, leaderboards_tint);

	ImGui::PopFont();
}

static bool perform_watch_button(const float h) {
	const auto label = "Watch on YouTube";
	const auto& style = ImGui::GetStyle();

	const auto icon_side = std::round(h * 0.4f);
	const auto icon_gap = std::round(icon_side * 0.6f);
	const auto contents_w = icon_side + icon_gap + ImGui::CalcTextSize(label).x;

	const auto row_w = ImGui::GetContentRegionAvail().x;
	const auto w = std::min(row_w, std::max(row_w / 3, contents_w + style.FramePadding.x * 4));

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + std::round((row_w - w) / 2));

	const auto pos = vec2(ImGui::GetCursorScreenPos());
	const bool clicked = ImGui::Button("##watch_dev_journal", ImVec2(w, h));

	const auto contents_lt = pos + vec2(std::round((w - contents_w) / 2), std::round((h - icon_side) / 2));
	const auto color = ImGui::GetColorU32(leaderboards_tint);
	auto& draw_list = *ImGui::GetWindowDrawList();

	draw_list.AddTriangleFilled(
		static_cast<ImVec2>(contents_lt),
		static_cast<ImVec2>(contents_lt + vec2(icon_side, icon_side / 2)),
		static_cast<ImVec2>(contents_lt + vec2(0.0f, icon_side)),
		color
	);

	const auto text_pos = vec2(
		contents_lt.x + icon_side + icon_gap,
		pos.y + std::round((h - ImGui::GetTextLineHeight()) / 2)
	);
	draw_list.AddText(static_cast<ImVec2>(text_pos), color, label);

	return clicked;
}

bool dev_journal_window_state::perform(const bool changelog_continues_in_the_same_popup) {
	using namespace augs::imgui;

	if (!show) {
		return false;
	}

	if (augs::imgui::is_other_popup_open(get_title())) {
		return false;
	}

	center_next_window(vec2(0.6f, 0.75f), ImGuiCond_Always);

	const auto flags =
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoDocking
	;

	auto imgui_window = scoped_modal_popup(get_title(), &show, flags);

	if (!imgui_window) {
		return !show;
	}

	const auto& style = ImGui::GetStyle();
	const auto larger_font = ImGui::GetIO().Fonts->Fonts[1];

	const auto button_h = (larger_font->FontSize + style.FramePadding.y * 2) * 1.4f;
	const auto header_h = std::round(larger_font->FontSize + style.ItemSpacing.y * 2);
	const auto buttons_h = button_h * 2 + style.ItemSpacing.y * 2;

	const auto content_lt = vec2(ImGui::GetCursorPos());
	const auto content_size = vec2(ImGui::GetContentRegionAvail());

	const auto thumbnail_margin = (content_size * vec2(0.1f, 0.05f)).discard_fract();
	const auto area_lt = content_lt + vec2(0.0f, header_h) + thumbnail_margin;
	const auto area_size = vec2(content_size - vec2(0.0f, header_h + buttons_h) - thumbnail_margin * 2).biggen(vec2::zero);

	::center_larger_text("NEW DEV JOURNAL!", content_lt, vec2(content_size.x, header_h));

	auto button_cols = scoped_button_colors({
		rgba(255, 255, 255, 45) * leaderboards_tint,
		rgba(255, 255, 255, 70) * leaderboards_tint,
		rgba(255, 255, 255, 100) * leaderboards_tint
	});

	const bool thumbnail_clicked = [&]() {
		if (thumbnail_size.has_value()) {
			return ::perform_dev_journal_thumbnail(*thumbnail_size, area_lt, area_size);
		}

		::center_larger_text(latest_dev_journal_v.title, area_lt, area_size);
		return false;
	}();

	ImGui::SetCursorPos(static_cast<ImVec2>(content_lt + vec2(0.0f, header_h + area_size.y + thumbnail_margin.y * 2)));

	const bool watch_clicked = ::perform_watch_button(button_h);

	ImGui::PushFont(larger_font);
	const bool ok_pressed = ImGui::Button("OK", ImVec2(-1, button_h));
	ImGui::PopFont();

	if (augs::is_ready(thumbnail_response)) {
		if (auto thumbnail = ::decode_dev_journal_thumbnail(augs::get_once(thumbnail_response)); thumbnail.has_value()) {
			thumbnail_size = thumbnail->get_size();
			thumbnail_to_upload = std::move(*thumbnail);
		}
	}

	const bool watched = thumbnail_clicked || watch_clicked;

	if (watched) {
		augs::open_url(latest_dev_journal_v.youtube_url);
	}

	const bool escape_pressed = ImGui::IsKeyPressed(ImGuiKey_Escape, false);

	if (watched || ok_pressed || escape_pressed || !show) {
		show = false;

		if (!changelog_continues_in_the_same_popup) {
			ImGui::CloseCurrentPopup();
		}
		return true;
	}

	return false;
}
