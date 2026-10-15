#pragma once
#include <algorithm>
#include <array>
#include <string>
#include "3rdparty/imgui/imgui.h"
#include "3rdparty/imgui/imgui_internal.h"
#include "augs/misc/scope_guard.h"

/*
	A text field for a long text of no spaces (e.g. a key): one line with the text shortened in the middle ("abc...xyz")
	while not active; clicked, it grows to as many lines as the whole text takes (broken at the field's width - the
	breaks are never part of the text), all of it selected; typed into, it wraps at once; Enter leaves it; no more
	characters than buffer_size - 1. Read-only with ImGuiInputTextFlags_ReadOnly.

	Its state lives in the window's storage by the label's id - nothing to keep at the call site.
*/

namespace augs {
	namespace imgui {
		/*
			The beginning and the end of a text with "..." between, as much as fits in width.
		*/

		inline std::string shortened_in_the_middle(const std::string& text, const float width) {
			if (ImGui::CalcTextSize(text.c_str()).x <= width) {
				return text;
			}

			for (std::size_t n = text.size() / 2; n > 1; --n) {
				auto shown = text.substr(0, n) + "..." + text.substr(text.size() - n);

				if (ImGui::CalcTextSize(shown.c_str()).x <= width) {
					return shown;
				}
			}

			return text;
		}

		namespace long_input_text_detail {
			enum class field_state {
				IDLE,
				FOCUS_NEXT,
				SELECTING,
				EXPANDED
			};

			inline ImGuiID state_id_of(const std::string& label) {
				return ImGui::GetID((label + "##long_input_state").c_str());
			}

			inline ImGuiID drawn_frame_id_of(const std::string& label) {
				return ImGui::GetID((label + "##long_input_drawn").c_str());
			}

			/*
				A field not drawn last frame (its window closed, another tab) is idle, whatever it was left as.
			*/

			inline field_state state_of(const std::string& label) {
				auto* const storage = ImGui::GetStateStorage();
				const bool drawn_last_frame = storage->GetInt(drawn_frame_id_of(label), -2) >= ImGui::GetFrameCount() - 1;

				if (!drawn_last_frame) {
					return field_state::IDLE;
				}

				return static_cast<field_state>(storage->GetInt(state_id_of(label), static_cast<int>(field_state::IDLE)));
			}
		}

		/*
			The two below look the field up by its label in the current window and id stack - call them where the field
			itself is drawn.

			Whether the field with this label is being edited (expanded) - e.g. to take a paste whole before it does.
		*/

		inline bool long_input_text_with_dots_active(const std::string& label) {
			using namespace long_input_text_detail;
			return state_of(label) != field_state::IDLE;
		}

		/*
			The field with this label gets the keyboard on the next frame, all of it selected.
		*/

		inline void long_input_text_with_dots_focus(const std::string& label) {
			using namespace long_input_text_detail;
			ImGui::GetStateStorage()->SetInt(state_id_of(label), static_cast<int>(field_state::FOCUS_NEXT));
		}

		template <std::size_t buffer_size>
		bool long_input_text_with_dots(
			const std::string& label,
			std::string& value,
			const ImGuiInputTextFlags flags = 0
		) {
			using namespace long_input_text_detail;

			auto* const storage = ImGui::GetStateStorage();
			const auto state_id = state_id_of(label);
			auto state = state_of(label);

			/*
				The state back into the window's storage whatever way this returns.
			*/

			auto store = augs::scope_guard([&]() {
				storage->SetInt(state_id, static_cast<int>(state));
				storage->SetInt(drawn_frame_id_of(label), ImGui::GetFrameCount());
			});

			const auto& style = ImGui::GetStyle();
			const auto width = ImGui::CalcItemWidth();
			const auto inner_width = width - 2 * style.FramePadding.x;

			auto to_buffer = [](const std::string& text) {
				auto buf = std::array<char, buffer_size * 2>();
				const auto n = std::min(text.size(), buf.size() - 1);
				std::copy_n(text.data(), n, buf.data());
				buf[n] = '\0';
				return buf;
			};

			/*
				The shortened field an id of its own - its text never carried over into the expanded one.
			*/

			if (state == field_state::IDLE) {
				auto buf = to_buffer(shortened_in_the_middle(value, inner_width));
				ImGui::InputText((label + "##shortened").c_str(), buf.data(), buf.size(), ImGuiInputTextFlags_ReadOnly);

				if (ImGui::IsItemActivated()) {
					state = field_state::FOCUS_NEXT;
				}

				return false;
			}

			/*
				Lines as wide as the field.
			*/

			auto wrapped = [inner_width](const std::string& text) {
				auto out = std::string();
				auto line = std::string();

				for (const auto c : text) {
					if (c == '\n') {
						continue;
					}

					auto longer = line + c;

					if (!line.empty() && ImGui::CalcTextSize(longer.c_str()).x > inner_width) {
						out += line + '\n';
						line = std::string(1, c);
					}
					else {
						line = std::move(longer);
					}
				}

				return out + line;
			};

			const auto shown = wrapped(value);
			const auto num_lines = 1 + static_cast<std::size_t>(std::count(shown.begin(), shown.end(), '\n'));
			const auto height = num_lines * ImGui::GetTextLineHeight() + 2 * style.FramePadding.y;
			auto buf = to_buffer(shown);

			/*
				Typed into: no more characters than the field takes (the line breaks do not count - what went over
				is dropped right before the cursor, where it was typed), wrapped anew at once, the cursor after the same
				character.
			*/

			auto rewrap_on_edit = [](ImGuiInputTextCallbackData* data) -> int {
				const auto& wrap = *static_cast<const decltype(wrapped)*>(data->UserData);
				auto text = std::string(data->Buf, data->BufTextLen);

				auto chars_before_cursor = 0;

				for (int i = 0; i < data->CursorPos; ++i) {
					chars_before_cursor += text[i] != '\n';
				}

				{
					auto chars = text;
					std::erase(chars, '\n');

					const auto max_chars = static_cast<int>(buffer_size) - 1;
					const auto over = static_cast<int>(chars.size()) - max_chars;

					if (over > 0) {
						const auto cut_from = std::max(0, chars_before_cursor - over);
						chars.erase(cut_from, chars_before_cursor - cut_from);
						chars.resize(std::min(chars.size(), static_cast<std::size_t>(max_chars)));
						chars_before_cursor = cut_from;
						text = chars;
						data->DeleteChars(0, data->BufTextLen);
						data->InsertChars(0, text.c_str());
					}
				}

				const auto new_text = wrap(text);

				if (new_text != text) {
					data->DeleteChars(0, data->BufTextLen);
					data->InsertChars(0, new_text.c_str());

					auto cursor = 0;

					for (int seen = 0; cursor < static_cast<int>(new_text.size()) && seen < chars_before_cursor; ++cursor) {
						seen += new_text[cursor] != '\n';
					}

					data->CursorPos = data->SelectionStart = data->SelectionEnd = cursor;
				}

				return 0;
			};

			if (state == field_state::FOCUS_NEXT) {
				ImGui::SetKeyboardFocusHere();
			}

			/*
				Copied (Ctrl+C, Ctrl+X) without the line breaks - they are only how it shows. What ImGui copies is
				taken before it does (a cut leaves no selection) and put into the clipboard anew after, never read
				back from it (some platforms' clipboards are written later).
			*/

			const auto copied_unbroken = [&]() {
				const auto& io = ImGui::GetIO();
				const auto field_id = ImGui::GetID(label.c_str());

				const bool copy_pressed =
					(io.KeyCtrl || io.KeySuper)
					&& (ImGui::IsKeyPressed(ImGuiKey_C, false) || ImGui::IsKeyPressed(ImGuiKey_X, false))
				;

				const bool copying =
					copy_pressed
					&& !(flags & ImGuiInputTextFlags_Password)
					&& ImGui::GetActiveID() == field_id
				;

				auto out = std::string();

				if (!copying) {
					return out;
				}

				if (const auto* const text_state = ImGui::GetInputTextState(field_id); text_state != nullptr) {
					const auto whole = !text_state->HasSelection();
					const auto from = whole ? 0 : std::min(text_state->GetSelectionStart(), text_state->GetSelectionEnd());
					const auto to = whole ? text_state->CurLenW : std::max(text_state->GetSelectionStart(), text_state->GetSelectionEnd());

					for (auto i = from; i < to; ++i) {
						if (const auto c = text_state->TextW[i]; c != '\n') {
							out += static_cast<char>(c);
						}
					}
				}

				return out;
			}();

			/*
				Enter leaves it as it leaves a field of one line (CtrlEnterForNewLine - and no line break is ever kept).
			*/

			const auto multiline_flags =
				(flags & ~ImGuiInputTextFlags_AutoSelectAll)
				| ImGuiInputTextFlags_NoHorizontalScroll
				| ImGuiInputTextFlags_CallbackEdit
				| ImGuiInputTextFlags_CtrlEnterForNewLine
			;

			const bool changed = ImGui::InputTextMultiline(
				label.c_str(),
				buf.data(),
				buf.size(),
				ImVec2(width, height),
				multiline_flags,
				rewrap_on_edit,
				&wrapped
			);

			if (!copied_unbroken.empty()) {
				ImGui::SetClipboardText(copied_unbroken.c_str());
			}

			/*
				What follows on the same line (a label, an icon) sits as by a field of one line - not a few pixels higher.
			*/

			ImGui::GetCurrentWindow()->DC.PrevLineTextBaseOffset = style.FramePadding.y;

			if (changed && !(flags & ImGuiInputTextFlags_ReadOnly)) {
				auto edited = std::string(buf.data());
				std::erase(edited, '\n');
				value = edited.substr(0, buffer_size - 1);
			}

			/*
				All of it selected once it is active, and for as long as the button that clicked it is held - ImGui
				selects all only in fields of one line, and the click's drag would move the cursor.
			*/

			if (state == field_state::FOCUS_NEXT && ImGui::IsItemActive()) {
				state = field_state::SELECTING;
			}

			if (state == field_state::SELECTING) {
				if (const auto text_state = ImGui::GetInputTextState(ImGui::GetItemID()); text_state != nullptr) {
					text_state->SelectAll();
					text_state->SelectedAllMouseLock = true;
				}

				if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
					state = field_state::EXPANDED;
				}
			}
			else if (state == field_state::EXPANDED && !ImGui::IsItemActive()) {
				state = field_state::IDLE;
			}

			return changed;
		}
	}
}
