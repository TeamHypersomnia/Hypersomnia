#include "application/gui/client/chat_gui.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "application/setups/client/client_vars.h"
#include "view/faction_view_settings.h"
#include "augs/gui/text/printer.h"
#include "augs/drawing/drawing.h"

void chat_gui_state::open_input_bar(const chat_target_type t) {
	show = true;
	target = t;

	ImGui::SetWindowFocus("ChatWindow");
}

bool chat_gui_state::perform_input_bar(const client_chat_settings& vars) {
	using namespace augs::imgui;

	if (!show) {
		return false;
	}

	const auto screen_size = vec2i(ImGui::GetIO().DisplaySize);

	const auto size = vec2 {
		static_cast<float>(vars.chat_window_width),
		ImGui::GetTextLineHeight() * 3.f
	};

	const auto window_offset = vars.chat_window_offset;
	const auto window_pos = vec2(window_offset.x, screen_size.y - size.y - window_offset.y);

	ImGui::SetNextWindowPos(ImVec2(window_pos));

	auto go_to_entity = scoped_window(
		"ChatWindow", 
		&show,
		ImGuiWindowFlags_NoTitleBar 
		| ImGuiWindowFlags_NoResize 
		| ImGuiWindowFlags_NoMove 
		| ImGuiWindowFlags_NoScrollbar 
		| ImGuiWindowFlags_NoScrollWithMouse
		| ImGuiWindowFlags_NoSavedSettings
		| ImGuiWindowFlags_AlwaysAutoResize
	);

	bool was_acquired = false;
	
	if (ImGui::GetCurrentWindow()->GetID("##ChatInput") != GImGui->ActiveId) {
		ImGui::SetKeyboardFocusHere();
		was_acquired = true;
	}

	auto scope = augs::scope_guard([&](){
		if (!was_acquired && ImGui::GetCurrentWindow()->GetID("##ChatInput") != GImGui->ActiveId) {
			show = false;
		}
	});

	std::string label;

	switch (target) {
		case chat_target_type::GENERAL: label = "(Say to all)"; break;
		case chat_target_type::TEAM_ONLY: label = "(Say to team)"; break;
		default: break;
	}

	text_disabled(label);

	ImGui::SameLine();

	{
		auto scope = augs::imgui::scoped_item_width(size.x);

		if (input_text<max_chat_message_length_v>("##ChatInput", current_message, ImGuiInputTextFlags_EnterReturnsTrue)) {
			show = false;

			if (current_message.size() > 0) {
				return true;
			}
		}
	}

	return false;
}

void chat_gui_state::draw_recent_messages(
	const augs::drawer drawer,
	const client_chat_settings& vars,
   	const faction_view_settings& faction_view,
	const augs::baked_font& gui_font,
   	const net_time_t current_time
) const {
	using namespace augs::gui::text;

	const auto entries_to_show = std::min(
		history.size(), 
		static_cast<std::size_t>(vars.show_recent_chat_messages_num)
	);

	const auto now = current_time;

	const auto starting_i = [&]() {
		auto i = history.size() - entries_to_show;

		while (i < history.size() && now - history[i].timestamp >= vars.keep_recent_chat_messages_for_seconds) {
			++i;
		}

		return static_cast<int>(i);
	}();

	auto get_col = [&](const auto& faction) {
		return faction_view.colors[faction].standard;
	};

	auto colored = [&](const auto& text, const auto& c) {
		const auto text_style = style(gui_font, c);
		return formatted_string(text, text_style);
	};

	const auto wrapping = vars.chat_window_width;

	auto calc_size = [&](const auto& text) { 
		return get_text_bbox(colored(text, white), wrapping);
	};

	const auto size = vec2 {
		static_cast<float>(vars.chat_window_width),
		ImGui::GetTextLineHeight() * 3.f
	};

	const auto window_offset = vars.chat_window_offset;
	const auto screen_size = vec2i(ImGui::GetIO().DisplaySize);
	const auto window_pos = vec2(window_offset.x, screen_size.y - size.y - window_offset.y);

	auto pen = window_pos;

	for (int i = history.size() - 1; i >= starting_i; --i) {
		const auto& entry = history[i];

		const auto author_text = entry.get_author_string();

		const auto author_col = get_col(entry.author_faction);
		const auto message_col = entry.overridden_message_color == rgba::zero ? white : entry.overridden_message_color;

		const auto total_text = colored(author_text, author_col) + colored(entry.message, message_col);

		const auto bbox = calc_size(total_text);
		pen.y -= bbox.y + 1;

		print_stroked(
			drawer,
			pen,
			total_text,
			augs::ralign_flags{},
			black,
			wrapping
		);
	}
}
