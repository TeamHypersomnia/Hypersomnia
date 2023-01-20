#include "augs/log.h"
#include "application/setups/debugger/gui/debugger_go_to_gui.h"

#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "3rdparty/imgui/imgui_internal.h"

#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"

#include "application/setups/debugger/debugger_view.h"
#include "application/setups/debugger/debugger_settings.h"
#include "game/cosmos/for_each_entity.h"

const_entity_handle debugger_go_to_entity_gui::get_matching_go_to_entity(const cosmos& cosm) const {
	if (last_input.empty() && !moved_since_opening) {
		return cosm[entity_id()];
	}

	if (show && matches.size() > 0) {
		return cosm[matches[selected_index]];
	}

	return cosm[entity_id()];
}

void debugger_go_to_entity_gui::open() {
	show = true;
	textbox_data.clear();
	matches.clear();
	last_input.clear();
	selected_index = 0;
	moved_since_opening = false;

	ImGui::SetWindowFocus("Go to entity");
}

void standard_confirm_go_to(const const_entity_handle match, const bool has_ctrl, debugger_view& view, debugger_view_ids& view_ids) {
	/* Confirm selection with quick search */

	if (match) {
		if (has_ctrl) {
			view_ids.selected_entities.emplace(match);
		}
		else {
			view_ids.selected_entities = { match };
		}

		if (!view.panned_camera.has_value()) {
			view.panned_camera = camera_eye();
		}

		if (const auto transform = match.find_logic_transform()) {
			view.panned_camera->transform.pos = transform->pos;
		}
		else {
			LOG("WARNING: transform of %x could not be found.", match);
		}
	}
}

struct text_edit_callback_input {
	const cosmos& cosm;
	debugger_go_to_entity_gui& self;
};

std::optional<const_entity_handle> debugger_go_to_entity_gui::perform(
	const debugger_go_to_settings& settings,
	const cosmos& cosm,
	const vec2 dialog_pos
) {
	if (!show) {
		return std::nullopt;
	}

	using namespace ImGui;
	using namespace augs::imgui;

	{
		const auto max_lines = static_cast<std::size_t>(settings.num_lines);
		const auto left = selected_index / max_lines * max_lines;
		const auto right = std::min(left + max_lines, matches.size());

		const auto size = vec2 {
			static_cast<float>(settings.dialog_width),
			(right - left + 2) * ImGui::GetTextLineHeightWithSpacing()
		};

		set_next_window_rect(
			{
				dialog_pos - vec2(size.x / 2, 0),
				size	
			},
			ImGuiCond_Always
		);
	}

	auto go_to_entity = scoped_window(
		"Go to entity", 
		&show,
	   	ImGuiWindowFlags_NoTitleBar 
		| ImGuiWindowFlags_NoResize 
		| ImGuiWindowFlags_NoScrollbar 
		| ImGuiWindowFlags_NoScrollWithMouse
		| ImGuiWindowFlags_NoMove 
		| ImGuiWindowFlags_NoSavedSettings
	);

	static auto arrow_callback = [](auto* data) {
		auto& input = *reinterpret_cast<text_edit_callback_input*>(data->UserData);
		auto& self = input.self;
		auto& cosm = input.cosm;

		switch (data->EventFlag) {
			case ImGuiInputTextFlags_CallbackHistory: {
				const auto max_index = self.matches.size();

				if (max_index == 0) {
					break;
				}
				
				if (data->EventKey == ImGuiKey_UpArrow) {
					self.moved_since_opening = true;

					if (self.selected_index == 0) {
						self.selected_index = max_index - 1;
					}
					else {
						--self.selected_index; 	
					}
				}
				else if (data->EventKey == ImGuiKey_DownArrow) {
					self.moved_since_opening = true;

					++self.selected_index; 	
					self.selected_index %= max_index;
				}
			}

			break;

			case ImGuiInputTextFlags_CallbackAlways: {
				const auto current_input_text = std::string(data->Buf, data->BufTextLen);
	
				const bool should_rebuild = 
					current_input_text != self.last_input
					|| (current_input_text.empty() && self.matches.empty())
				;
	
				if (!should_rebuild) {
					break;
				}
	
				self.last_input = current_input_text;
	
				self.matches.clear();
				const auto query = current_input_text;
	
				unsigned hits = 0;
				(void)hits;
					
				cosm.for_each_entity([&](const auto& handle) {
					const auto name = handle.get_name();
	
					if (query.empty() || to_lowercase(name).find(query) != std::string::npos) {
						++hits;
	
						self.matches.push_back(handle.get_id());	
					}
				});	

				if (self.matches.size() > 0) {
					self.selected_index %= self.matches.size();
				}
				else {
					self.selected_index = 0;
				}
			}
			break;
			
			case ImGuiInputTextFlags_CallbackCompletion: {
				if (const auto match = self.get_matching_go_to_entity(cosm)) {
					const auto name = match.get_name();

					if (static_cast<int>(name.length()) < data->BufSize) {
						std::copy(name.c_str(), name.c_str() + name.length() + 1, data->Buf);

						data->BufTextLen = name.length();
						data->CursorPos = name.length();
						data->BufDirty = true;
					}
				}

				break;
			}

			default: break;
		}

		return 0;
	};

	text("Go to entity");
	ImGui::SameLine();
	
	bool was_acquired = false;
	
	if (ImGui::GetCurrentWindow()->GetID("##GoToEntityInput") != GImGui->ActiveId) {
		ImGui::SetKeyboardFocusHere();
		was_acquired = true;
	}
	
	text_edit_callback_input input {
		cosm,
		*this
	};

	auto scope = augs::scope_guard([&](){
		if (!was_acquired && ImGui::GetCurrentWindow()->GetID("##GoToEntityInput") != GImGui->ActiveId) {
			show = false;
		}
	});

	if (input_text<256>(
		"##GoToEntityInput", 
		textbox_data,
			ImGuiInputTextFlags_CallbackHistory 
			| ImGuiInputTextFlags_EnterReturnsTrue 
			| ImGuiInputTextFlags_CallbackAlways
			| ImGuiInputTextFlags_CallbackCompletion
		, 
		arrow_callback,
		reinterpret_cast<void*>(&input)
	)) {
		return get_matching_go_to_entity(cosm);
	}

	{
		const auto query = last_input;

		const auto max_lines = static_cast<std::size_t>(settings.num_lines);
		const auto left = selected_index / max_lines * max_lines;
		const auto right = std::min(left + max_lines, matches.size());

		for (std::size_t i = left; i < right; ++i) {
			if (i == selected_index) {
				bool s = true;
				ImGui::PushID(static_cast<int>(i));
				ImGui::Selectable("##gotoentity", &s);
				ImGui::PopID();
				ImGui::SameLine(0.f, 0.f);
			}

			const auto m = matches[i];
			const auto name = cosm[m].get_name();
			const auto matched_name = to_lowercase(name);

			const auto unmatched_left = matched_name.find(query);
			const auto unmatched_right = unmatched_left + query.length();

			text(name.substr(0, unmatched_left));
			ImGui::SameLine(0.f, 0.f);

			text_color(name.substr(unmatched_left, unmatched_right - unmatched_left), green);
			ImGui::SameLine(0.f, 0.f);

			text(name.substr(unmatched_right));

			ImGui::SameLine(settings.dialog_width * 0.7);

			text("(id: %x)", m.raw);
		}
	}

	return std::nullopt;
}
