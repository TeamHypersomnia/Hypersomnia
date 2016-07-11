#pragma once
#include <vector>
#include "math/vec2.h"
#include "window_framework/event.h"
#include "misc/timer.h"
#include "graphics/pixel.h"
#include "texture_baker/texture_baker.h"
#include "texture_baker/font.h"
#include "rect.h"

#include <functional>
#include "misc/object_pool.h"

#include "game/assets/font_id.h"

namespace augs {
	namespace gui {
		namespace text {
			struct formatted_char {
				assets::font_id font_used;
				wchar_t c;
				unsigned char r, g, b, a;
				void set(wchar_t, assets::font_id = assets::font_id::GUI_FONT, const rgba& = rgba());
				void set(assets::font_id = assets::font_id::GUI_FONT, const rgba& = rgba());

				bool operator==(const formatted_char& b) {
					return font_used == b.font_used && c == b.c;
				}
			};

			struct style {
				assets::font_id f;
				rgba color;
				style(assets::font_id = assets::font_id::GUI_FONT, rgba = rgba());
				style(const formatted_char&);
				operator formatted_char();
			};

			typedef std::basic_string<formatted_char> fstr;

			std::wstring formatted_string_to_wstring(const fstr& f);
			fstr simple_bbcode(std::wstring, style);
			fstr format(const std::wstring&, style);
			void format(const std::wstring&, style, fstr&);
			void format(const wchar_t*, style, fstr&);
			fstr format(const wchar_t*, style);
		}

		class gui_world {
			float delta_ms = 1000 / 60.f;

		public:
			rect_pool rects;

			class clipboard {
				bool
					/* own_copy indicates whether the clipboard_change event comes from manual "copy_clipboard" or from external source */
					own_copy = false,
					own_clip = false;

			public:
				bool fetch_clipboard = true;

				text::fstr contents;

				void change_clipboard();
				void copy_clipboard(text::fstr&);

				bool is_clipboard_own() const;
			};

			struct middlescroll_data {
				material mat;
				rects::wh<float> size = rects::wh<float>(25, 25);
				vec2i pos;
				rect_id subject;
				float speed_mult = 1.f;
			};

			static clipboard global_clipboard;

			rect_id rect_in_focus;

			middlescroll_data middlescroll;

			augs::window::event::state state;

			bool was_hovered_rect_visited = false;
			bool held_rect_is_dragged = false;
			rect_id rect_hovered;
			rect_id rect_held_by_lmb;
			rect_id rect_held_by_rmb;
			
			vec2i ldrag_relative_anchor;
			vec2i last_ldown_position;
			vec2i current_drag_amount;

			rect_id root;

			gui_world();

			void set_delta_milliseconds(float);
			float delta_milliseconds();

			void set_focus(rect_id, std::function<void(rect_handle, )> event_behaviour);
			rect_id get_rect_in_focus() const;

			void consume_raw_input_and_generate_gui_events(augs::window::event::state);
			void perform_logic_step();
			vertex_triangle_buffer draw_triangles() const;
		};

		void paste_clipboard_formatted(text::fstr& out, text::formatted_char = text::formatted_char());
	}
}

