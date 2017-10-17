#pragma once
#include <imgui/imgui.h>

#include "augs/misc/machine_entropy.h"

namespace augs {
	class image;

	namespace graphics {
		class texture;
	}

	namespace imgui {
		void init(
			const char* const ini_filename,
			const char* const log_filename
		);

		image create_atlas_image();
		graphics::texture create_atlas();

		void setup_input(
			local_entropy& window_inputs,
			const decltype(ImGuiIO::DeltaTime) delta_seconds,
			const vec2i screen_size
		);

		void setup_input(
			event::state& state,
			const decltype(ImGuiIO::DeltaTime) delta_seconds,
			const vec2i screen_size
		);

		void neutralize_mouse();

		void render(const ImGuiStyle&);
	
		template <class id_type>
		id_type get_cursor() {
			if (ImGui::GetMouseCursor() == ImGuiMouseCursor_TextInput) {
				return id_type::GUI_CURSOR_TEXT_INPUT;
			}

			if (ImGui::GetMouseCursor() == ImGuiMouseCursor_ResizeNWSE) {
				return id_type::GUI_CURSOR_RESIZE_NWSE;
			}

			if (ImGui::IsAnyItemHoveredWithHandCursor()) {
				return id_type::GUI_CURSOR_HOVER;
			}

			return id_type::GUI_CURSOR;
		}

		inline auto filter_inputs(augs::local_entropy local) {
			const bool filter_mouse = ImGui::GetIO().WantCaptureMouse;
			const bool filter_keyboard = ImGui::GetIO().WantTextInput;

			using namespace augs::event;
			using namespace keys;

			erase_if(local, [filter_mouse, filter_keyboard](const change ch) {
				if (filter_mouse && ch.msg == message::mousemotion) {
					return true;
				}

				/* We always let release events propagate */

				if (ch.was_any_key_pressed()) {
					if (filter_mouse && ch.uses_mouse()) {
						return true;
					}

					if (filter_keyboard && ch.uses_keyboard()) {
						if (ch.is_shortcut_key()) {
							return false;
						}

						return true;
					}
				}

				return false;
			});

			return local;
		}
	}
}