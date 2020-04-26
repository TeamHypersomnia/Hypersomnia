#pragma once
#include "3rdparty/imgui/imgui.h"

#include "augs/templates/container_templates.h"
#include "augs/misc/machine_entropy.h"
#include "augs/templates/exception_templates.h"

namespace augs {
	class image;
	struct font_loading_input;

	struct imgui_init_error : error_with_typesafe_sprintf {
		using error_with_typesafe_sprintf::error_with_typesafe_sprintf;
	};

	namespace graphics {
		class texture;
	}

	namespace imgui {
		template <class C, class R>
		void concat_ranges(R& ranges, const C* const ranges_array) {
			for (auto* p = ranges_array; *p; p += 2) {
				ranges.push_back({ p[0], p[1] });
			}
		}

		class context_raii { 
		public:
			context_raii(
				const char* const ini_filename,
				const char* const log_filename,
				const ImGuiStyle& initial_style
			);

			~context_raii();

			context_raii(const context_raii&) = delete;
			context_raii& operator=(const context_raii&) = delete;

			context_raii(context_raii&&) = delete;
			context_raii& operator=(context_raii&&) = delete;
		};

		image create_atlas_image(const font_loading_input&);
		graphics::texture create_atlas(const font_loading_input&);

		void setup_io_settings(
			const decltype(ImGuiIO::DeltaTime) delta_seconds,
			const vec2i screen_size
		);

		void pass_inputs(
			local_entropy& window_inputs
		);

		void neutralize_mouse();

		void render();
	
		bool is_hovered_with_hand_cursor();

		template <class id_type>
		id_type get_cursor() {
			if (ImGui::GetMouseCursor() == ImGuiMouseCursor_TextInput) {
				return id_type::GUI_CURSOR_TEXT_INPUT;
			}

			if (ImGui::GetMouseCursor() == ImGuiMouseCursor_ResizeNWSE) {
				return id_type::GUI_CURSOR_RESIZE_NWSE;
			}

			if (is_hovered_with_hand_cursor()) {
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

				if (filter_mouse && ch.msg == message::wheel) {
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

		void center_next_window(ImGuiCond);
		void center_next_window(vec2 size_multiplier, ImGuiCond);
		void set_next_window_rect(xywh, ImGuiCond = ImGuiCond_FirstUseEver);
	}
}