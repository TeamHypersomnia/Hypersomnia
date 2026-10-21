#include <cmath>
#include <algorithm>
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_game_image.h"
#include "augs/window_framework/shell.h"
#include "augs/misc/date_time.h"
#include "augs/string/typesafe_sprintf.h"
#include "hypersomnia_version.h"
#include "application/gui/changelog_gui.h"
#include "application/gui/leaderboards_gui.h"

rgba get_new_feature_color() {
	return rgba::get_bright_wave(augs::steady_secs() / 4.0, 0.55);
}

std::string get_newest_changelog_version(const changelog_entries& entries) {
	if (entries.empty()) {
		return "";
	}

	return entries.front().version;
}

bool should_show_changelog(const changelog_entries& entries, const std::string& last_seen_version) {
#if !IS_PRODUCTION_BUILD
	/*
		Developers want their leaderboards - the changelog opens only from its button or Settings.
	*/

	(void)entries;
	(void)last_seen_version;
	return false;
#else
	const auto newest = ::get_newest_changelog_version(entries);

	if (newest.empty()) {
		return false;
	}

	if (last_seen_version.empty()) {
		return true;
	}

	return ::is_more_recent(newest, last_seen_version);
#endif
}

/*
	The changelog opens on the oldest release not acknowledged yet - a big one is read first, the newer ones above it
	are (NEW!) too. Nothing acknowledged at all ("") or everything: the newest.
*/

std::size_t oldest_unseen_changelog_index(const changelog_entries& entries, const std::string& last_seen_version) {
	if (last_seen_version.empty()) {
		return 0;
	}

	auto result = std::size_t(0);

	for (std::size_t i = 0; i < entries.size(); ++i) {
		if (::is_more_recent(entries[i].version, last_seen_version)) {
			result = i;
		}
	}

	return result;
}

static bool is_new_version(
	const changelog_input& in,
	const changelog_entry& entry,
	const std::size_t index
) {
	/*
		The newest release is always (NEW!) - it is what the changelog is shown for.
	*/

	if (index == 0) {
		return true;
	}

	if (in.state.read_versions.count(entry.version) > 0) {
		return false;
	}

#if !IS_PRODUCTION_BUILD
	/*
		Development builds mark the two newest versions (NEW!) to preview how a release will look.
	*/

	return index < 2;
#else
	/*
		A fresh config (empty) has seen nothing, but only the newest is (NEW!) then - is_more_recent is false for it.
	*/

	return ::is_more_recent(entry.version, in.last_seen_version);
#endif
}

static std::string format_days_ago(const int64_t timestamp) {
	const auto secs_ago = augs::date_time(static_cast<std::time_t>(timestamp)).seconds_ago();

	/*
		Dates have no time of day, so hours ago would be made up.
	*/

	if (secs_ago < 60 * 60 * 24) {
		return "Today";
	}

	return augs::date_time::format_how_long_ago(false, secs_ago);
}

static rgba get_heading_color(const std::string& heading) {
	if (heading == "Added") {
		return rgba(140, 225, 140, 255);
	}

	if (heading == "Changed") {
		return leaderboards_tint;
	}

	if (heading == "Fixed") {
		return rgba(140, 195, 255, 255);
	}

	if (heading == "Removed") {
		return rgba(255, 140, 140, 255);
	}

	return white;
}

static void draw_token(const std::string& token, const augs::markdown_span_style style, const rgba base_col) {
	using namespace augs::imgui;

	auto draw_list = ImGui::GetWindowDrawList();

	switch (style) {
		case augs::markdown_span_style::BOLD: {
			/*
				No bold font is loaded, so the text is drawn twice, 1px apart.
			*/

			const auto bold_col = rgba(255, 238, 205, 255);
			const auto pos = ImGui::GetCursorScreenPos();

			{
				auto sc = scoped_text_color(bold_col);
				ImGui::TextUnformatted(token.c_str());
			}

			draw_list->AddText(ImVec2(pos.x + 1, pos.y), ImGui::GetColorU32(bold_col), token.c_str());
			break;
		}

		case augs::markdown_span_style::ITALIC: {
			auto sc = scoped_text_color(rgba(205, 205, 255, 255));
			ImGui::TextUnformatted(token.c_str());
			break;
		}

		case augs::markdown_span_style::CODE: {
			const auto pos = vec2(ImGui::GetCursorScreenPos());
			const auto size = vec2(ImGui::CalcTextSize(token.c_str()));
			const auto pad = vec2(2, 0);

			draw_list->AddRectFilled(
				static_cast<ImVec2>(pos - pad),
				static_cast<ImVec2>(pos + size + pad),
				ImGui::GetColorU32(rgba(255, 255, 255, 22)),
				3.0f
			);

			auto sc = scoped_text_color(rgba(255, 200, 200, 255));
			ImGui::TextUnformatted(token.c_str());
			break;
		}

		default: {
			auto sc = scoped_text_color(base_col);
			ImGui::TextUnformatted(token.c_str());
			break;
		}
	}
}

/*
	Word-wraps a line of mixed styles.
	Wrapped lines start at the x the line started at, giving bullets a hanging indent.
*/

/*
	Table cells don't wrap - their columns fit the widest cell instead.
*/

static void draw_spans(const std::vector<augs::markdown_span>& spans, const rgba base_col, const bool wrap = true) {
	const auto start_x = ImGui::GetCursorPosX();
	/*
		Wrapped a bit before the edge so the text keeps away from the scrollbar.
	*/

	const auto wrap_right = ImGui::GetContentRegionMax().x - ImGui::GetFontSize();

	bool line_empty = true;

	for (const auto& span : spans) {
		const auto& text = span.text;
		const bool bold = span.style == augs::markdown_span_style::BOLD;

		std::size_t i = 0;

		while (i < text.size()) {
			auto j = i;

			while (j < text.size() && text[j] != ' ') {
				++j;
			}

			const auto word_end = j;

			while (j < text.size() && text[j] == ' ') {
				++j;
			}

			const auto token = text.substr(i, j - i);
			const auto word = text.substr(i, word_end - i);

			i = j;

			if (!line_empty) {
				ImGui::SameLine(0.0f, 0.0f);

				const auto word_w = ImGui::CalcTextSize(word.c_str()).x + (bold ? 1.0f : 0.0f);

				if (wrap && ImGui::GetCursorPosX() + word_w > wrap_right) {
					ImGui::NewLine();
					ImGui::SetCursorPosX(start_x);

					if (word.empty()) {
						continue;
					}
				}
			}

			::draw_token(token, span.style, base_col);
			line_empty = false;
		}
	}

	if (line_empty) {
		ImGui::NewLine();
	}
}

static void draw_entry(const changelog_entry& entry, changelog_view_state& state) {
	using namespace augs::imgui;
	using L = augs::markdown_line_type;

	const auto larger_font = ImGui::GetIO().Fonts->Fonts[1];
	const auto& style = ImGui::GetStyle();
	const auto text_col = rgba(style.Colors[ImGuiCol_Text]);

	{
		ImGui::PushFont(larger_font);
		text_color("Hypersomnia " + entry.version, leaderboards_tint);
		ImGui::PopFont();
	}

	if (!entry.date.empty()) {
		text_disabled(typesafe_sprintf("%x (%x)", entry.date, ::format_days_ago(entry.timestamp)));
	}

	ImGui::Separator();

	const auto indent_w = ImGui::GetFontSize() * 1.2f;
	const auto& lines = entry.lines;

	/*
		Extra space above and below each top-level point.
	*/

	const auto row_pad = std::round(ImGui::GetFontSize() * 0.15f);

	enum class marker_type {
		DOT,
		PLUS,
		MINUS
	};

	/*
		Markers are drawn by hand in a fixed-width column so that every kind of row has the same height
		(ImGui::Bullet adds the frame padding to its row) and the text starts at the same x.
	*/

	auto draw_marked_line = [&](const augs::markdown_line& line, const marker_type marker, const rgba col) {
		auto draw_list = ImGui::GetWindowDrawList();
		const auto pos = ImGui::GetCursorScreenPos();
		const auto font_h = ImGui::GetFontSize();
		const auto col_u32 = ImGui::GetColorU32(col);

		if (marker == marker_type::DOT) {
			draw_list->AddCircleFilled(ImVec2(pos.x + font_h * 0.5f, pos.y + font_h * 0.5f), font_h * 0.12f, col_u32);
		}
		else {
			const auto plus_w = ImGui::CalcTextSize("+").x;
			const auto glyph_pos = ImVec2(pos.x + (font_h - plus_w) * 0.5f, pos.y);

			if (marker == marker_type::PLUS) {
				draw_list->AddText(glyph_pos, col_u32, "+");
			}
			else {
				/*
					The font's minus is narrower than its plus - drawn as a bar spanning the plus's horizontal stroke,
					as thick as the minus glyph.
				*/

				const auto font = ImGui::GetFont();
				const auto scale = font_h / font->FontSize;

				if (const auto plus = font->FindGlyph('+'); plus != nullptr) {
					if (const auto minus = font->FindGlyph('-'); minus != nullptr) {
						const auto mid_y = (plus->Y0 + plus->Y1) * 0.5f * scale;
						const auto half_thickness = (minus->Y1 - minus->Y0) * 0.5f * scale;

						draw_list->AddRectFilled(
							ImVec2(glyph_pos.x + plus->X0 * scale, glyph_pos.y + mid_y - half_thickness),
							ImVec2(glyph_pos.x + plus->X1 * scale, glyph_pos.y + mid_y + half_thickness),
							col_u32
						);
					}
				}
			}
		}

		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + font_h * 1.1f);
		::draw_spans(line.spans, col);
	};

	auto draw_bullet = [&](const augs::markdown_line& line) {
		const auto total_indent = indent_w * line.level;
		const auto bullet_col = line.level > 0 ? rgba(text_col).mult_alpha(0.8f) : text_col;

		if (total_indent > 0.0f) {
			ImGui::Indent(total_indent);
		}

		draw_marked_line(line, marker_type::DOT, bullet_col);

		if (total_indent > 0.0f) {
			ImGui::Unindent(total_indent);
		}
	};

	/*
		A top-level bullet with sub-points: + (collapsed) or - (expanded) instead of the dot, toggled by a click anywhere in its row.
		The selectable spans the whole row including the spacing below it, so neighbouring rows leave no gaps.
		The text is drawn first (on the upper draw channel) to know the row's height.
	*/

	auto draw_expandable = [&](const augs::markdown_line& line, const std::string& key) {
		const bool open = state.expanded_items.count(key) > 0;

		auto draw_list = ImGui::GetWindowDrawList();
		const auto row_start = ImGui::GetCursorPos();

		draw_list->ChannelsSplit(2);
		draw_list->ChannelsSetCurrent(1);

		ImGui::SetCursorPosY(row_start.y + row_pad);
		draw_marked_line(line, open ? marker_type::MINUS : marker_type::PLUS, text_col);

		const auto row_h = ImGui::GetCursorPosY() + row_pad - row_start.y;

		draw_list->ChannelsSetCurrent(0);
		ImGui::SetCursorPos(row_start);

		const bool clicked = [&]() {
			auto no_spacing = scoped_style_var(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, 0.0f));

			auto cols = scoped_selectable_colors({
				rgba(0, 0, 0, 0),
				rgba(255, 255, 255, 20) * leaderboards_tint,
				rgba(255, 255, 255, 35) * leaderboards_tint
			});

			return ImGui::Selectable(("##" + key).c_str(), false, 0, ImVec2(0, row_h));
		}();

		draw_list->ChannelsMerge();

		if (clicked) {
			if (open) {
				state.expanded_items.erase(key);
			}
			else {
				state.expanded_items.insert(key);
			}
		}

		return open;
	};

	/*
		A light line between the top-level points of a section, in the middle of the spacing above the row.
	*/

	auto draw_light_separator = [&]() {
		const auto y = ImGui::GetCursorScreenPos().y - style.ItemSpacing.y * 0.5f;
		const auto left = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMin().x;
		const auto right = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

		ImGui::GetWindowDrawList()->AddLine(
			ImVec2(left, y),
			ImVec2(right, y),
			ImGui::GetColorU32(rgba(255, 255, 255, 14) * leaderboards_tint)
		);
	};

	/*
		Consecutive table rows from begin as one table, indented to start where the text of a bullet at their level would.
		Returns the end of the rows.
	*/

	auto draw_table = [&](const std::size_t begin, const std::string& key) {
		const auto level = lines[begin].level;
		auto end = begin;
		auto num_columns = std::size_t(0);

		while (end < lines.size() && lines[end].type == L::TABLE_ROW && lines[end].level == level) {
			num_columns = std::max(num_columns, lines[end].cells.size());
			++end;
		}

		if (num_columns == 0) {
			return end;
		}

		const auto total_indent = indent_w * level + ImGui::GetFontSize() * 1.1f;

		ImGui::Indent(total_indent);

		const auto flags =
			ImGuiTableFlags_SizingFixedFit
			| ImGuiTableFlags_NoHostExtendX
			| ImGuiTableFlags_RowBg
			| ImGuiTableFlags_BordersInnerH
		;

		auto cell_padding = scoped_style_var(ImGuiStyleVar_CellPadding, ImVec2(ImGui::GetFontSize() * 0.6f, ImGui::GetFontSize() * 0.15f));

		if (ImGui::BeginTable(("##" + key).c_str(), static_cast<int>(num_columns), flags)) {
			for (auto r = begin; r < end; ++r) {
				const auto& row = lines[r];
				const auto col = row.is_header ? rgba(style.Colors[ImGuiCol_TextDisabled]) : text_col;

				ImGui::TableNextRow();

				for (std::size_t c = 0; c < row.cells.size(); ++c) {
					ImGui::TableSetColumnIndex(static_cast<int>(c));
					::draw_spans(row.cells[c], col, false);
				}
			}

			ImGui::EndTable();
		}

		ImGui::Unindent(total_indent);

		return end;
	};

	auto is_child_of = [&](const std::size_t candidate, const augs::markdown_line& parent) {
		const auto& child = lines[candidate];
		return (child.type == L::BULLET || child.type == L::TABLE_ROW) && child.level > parent.level;
	};

	bool first_heading = true;
	bool first_in_section = true;

	std::size_t i = 0;

	while (i < lines.size()) {
		const auto& line = lines[i];

		switch (line.type) {
			case L::HEADING: {
				const auto heading = line.get_plain_text();

				ImGui::Spacing();
				ImGui::Spacing();

				/*
					The first section follows the separator under the date.
				*/

				if (!first_heading) {
					ImGui::Separator();
					ImGui::Spacing();
				}

				first_heading = false;
				first_in_section = true;

				ImGui::PushFont(larger_font);
				text_color(heading, ::get_heading_color(heading));
				ImGui::PopFont();

				ImGui::Spacing();
				++i;
				break;
			}

			case L::BULLET: {
				auto children_end = i + 1;

				while (children_end < lines.size() && is_child_of(children_end, line)) {
					++children_end;
				}

				if (line.level == 0) {
					if (!first_in_section) {
						draw_light_separator();
					}

					first_in_section = false;
				}

				if (const bool expandable = line.level == 0 && children_end > i + 1) {
					const auto key = entry.version + "#" + std::to_string(i);

					if (draw_expandable(line, key)) {
						auto c = i + 1;

						while (c < children_end) {
							if (lines[c].type == L::TABLE_ROW) {
								c = draw_table(c, key + "#table" + std::to_string(c));
							}
							else {
								draw_bullet(lines[c]);
								++c;
							}
						}
					}

					i = children_end;
				}
				else if (line.level == 0) {
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + row_pad);
					draw_bullet(line);
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + row_pad);
					++i;
				}
				else {
					draw_bullet(line);
					++i;
				}

				break;
			}

			case L::TABLE_ROW: {
				ImGui::Spacing();
				i = draw_table(i, entry.version + "#table" + std::to_string(i));
				ImGui::Spacing();
				break;
			}

			default: {
				ImGui::Spacing();
				::draw_spans(line.spans, text_col);
				ImGui::Spacing();
				++i;
				break;
			}
		}
	}
}

/*
	The thumb with a line beside it - a fifth of the row (a quarter of OK), wider only if the text needs it.
	Asks for a review, not a positive one (Steamworks rules).
*/

static bool perform_steam_review_button(const necessary_images_in_atlas_map* const necessary_images, const float h) {
	using namespace augs::imgui;

	const auto& style = ImGui::GetStyle();

	const auto label = "Steam reviews help us grow.";

	const auto icon_side = std::round(h * 0.4f);
	const auto icon_gap = std::round(icon_side * 0.4f);
	const auto contents_w = icon_side + icon_gap + ImGui::CalcTextSize(label).x;

	const auto row_w = ImGui::GetContentRegionAvail().x;
	const auto w = std::max((row_w - style.ItemSpacing.x) / 5, contents_w + style.FramePadding.x * 4);

	const auto pos = vec2(ImGui::GetCursorScreenPos());

	const bool clicked = ImGui::Button("##steam_review", ImVec2(w, h));

	if (clicked) {
		augs::open_url("https://store.steampowered.com/recommended/recommendgame/2660970");
	}

	tooltip_on_hover("Want more people to play with?\nA review on Steam helps new players find the game.");

	const auto contents_lt = pos + vec2(std::round((w - contents_w) / 2), 0.0f);

	if (necessary_images != nullptr) {
		const auto& entry = (*necessary_images)[assets::necessary_image_id::THUMB_UP_ICON];

		auto cursor = scoped_preserve_cursor();
		ImGui::SetCursorScreenPos(static_cast<ImVec2>(contents_lt + vec2(0.0f, std::round((h - icon_side) / 2))));
		game_image(entry, vec2::square(icon_side), leaderboards_tint, vec2::zero, augs::imgui_atlas_type::GAME);
	}

	const auto text_pos = ImVec2(
		contents_lt.x + icon_side + icon_gap,
		pos.y + std::round((h - ImGui::GetTextLineHeight()) / 2)
	);

	ImGui::GetWindowDrawList()->AddText(text_pos, ImGui::GetColorU32(leaderboards_tint), label);

	return clicked;
}

/*
	Fills the remaining space of the current window. Returns true when OK was pressed.
*/

static bool perform_changelog_view(
	const changelog_input in,
	const necessary_images_in_atlas_map* const necessary_images,
	bool& steam_review_opened
) {
	using namespace augs::imgui;

	const auto& entries = in.entries;
	auto& state = in.state;

	const auto& style = ImGui::GetStyle();
	const auto larger_font = ImGui::GetIO().Fonts->Fonts[1];

	const auto ok_h = (larger_font->FontSize + style.FramePadding.y * 2) * 1.4f;
	const auto content_h = ImGui::GetContentRegionAvail().y - ok_h - style.ItemSpacing.y;

	if (entries.empty()) {
		auto child = scoped_child("changelog contents", ImVec2(0, content_h), false);
		text_disabled("No changelog available.");
	}
	else {
		if (state.selected >= entries.size()) {
			state.selected = 0;
		}

		bool selection_changed = false;

		{
			const auto versions_w = ImGui::CalcTextSize("99.99.99 (NEW!)").x + style.WindowPadding.x * 2 + style.ScrollbarSize;

			auto bg = scoped_style_color(ImGuiCol_ChildBg, rgba(255, 255, 255, 8) * leaderboards_tint);
			auto child = scoped_child("changelog versions", ImVec2(versions_w, content_h), false);

			auto selectable_cols = scoped_selectable_colors({
				rgba(255, 255, 255, 18) * leaderboards_tint,
				rgba(255, 255, 255, 30) * leaderboards_tint,
				rgba(255, 255, 255, 50) * leaderboards_tint
			});

			const auto new_version_col = ::get_new_feature_color();

			for (std::size_t i = 0; i < entries.size(); ++i) {
				const auto& entry = entries[i];
				const bool is_new = ::is_new_version(in, entry, i);

				/*
					Two lines: the version, and how long ago it came out in gray.
					The selectable is unlabeled and the text is drawn over it.
				*/

				const auto item_h = ImGui::GetTextLineHeight() * 2 + style.ItemSpacing.y;
				const auto item_pos = ImGui::GetCursorPos();

				if (ImGui::Selectable(("##" + entry.version).c_str(), state.selected == i, 0, ImVec2(0, item_h))) {
					if (state.selected != i) {
						/*
							A version loses its (NEW!) once it is left, not while it is being read.
						*/

						state.read_versions.insert(entries[state.selected].version);
						state.selected = i;
						selection_changed = true;
					}
				}

				const auto after_item = ImGui::GetCursorPos();

				ImGui::SetCursorPos(item_pos);
				text_color(entry.version + (is_new ? " (NEW!)" : ""), is_new ? new_version_col : rgba(style.Colors[ImGuiCol_Text]));

				if (!entry.date.empty()) {
					text_disabled(::format_days_ago(entry.timestamp));
				}

				ImGui::SetCursorPos(after_item);
			}
		}

		ImGui::SameLine();

		{
			auto child = scoped_child("changelog contents", ImVec2(0, content_h), false);

			if (selection_changed) {
				ImGui::SetScrollY(0.0f);
			}

			::draw_entry(entries[state.selected], state);
		}
	}

	auto button_cols = scoped_button_colors({
		rgba(255, 255, 255, 45) * leaderboards_tint,
		rgba(255, 255, 255, 70) * leaderboards_tint,
		rgba(255, 255, 255, 100) * leaderboards_tint
	});

	if (in.show_steam_review) {
		if (::perform_steam_review_button(necessary_images, ok_h)) {
			steam_review_opened = true;
		}

		ImGui::SameLine();
	}

	ImGui::PushFont(larger_font);
	const bool ok_pressed = ImGui::Button("OK", ImVec2(-1, ok_h));
	ImGui::PopFont();

	return ok_pressed;
}

/*
	Any open popup other than the changelog's own - e.g. the damaged account key, sign-in or an error.
*/

static bool is_other_popup_open(const std::string& own_title) {
	const auto own_id = ImGui::GetID(own_title.c_str());

	/*
		Only popups still drawn count - one whose window is no longer submitted (e.g. hidden by the changelog itself)
		stays in the stack and would make the changelog wait for good.
	*/

	for (const auto& popup : ImGui::GetCurrentContext()->OpenPopupStack) {
		const bool still_drawn = popup.Window != nullptr && popup.Window->WasActive;

		if (popup.PopupId != own_id && still_drawn) {
			return true;
		}
	}

	return false;
}

bool changelog_window_state::perform(const changelog_input in) {
	using namespace augs::imgui;

	if (!show) {
		return false;
	}

	/*
		Other modals take priority - the changelog waits until they close.
		Opening it over them would also close them, as both open from the root level.
	*/

	if (::is_other_popup_open(get_title())) {
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

	/*
		OK, X and Escape all acknowledge it.
	*/

	auto imgui_window = scoped_modal_popup(get_title(), &show, flags);

	if (!imgui_window) {
		return !show;
	}

	const bool ok_pressed = ::perform_changelog_view(in, necessary_images, steam_review_opened);
	const bool escape_pressed = ImGui::IsKeyPressed(ImGuiKey_Escape);

	if (ok_pressed || escape_pressed || !show) {
		show = false;
		ImGui::CloseCurrentPopup();
		return true;
	}

	return false;
}
