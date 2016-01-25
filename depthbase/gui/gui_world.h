#pragma once
#include <vector>
#include "math/vec2.h"
#include "window_framework/event.h"
#include "misc/timer.h"
#include "graphics/pixel.h"
#include "texture_baker/texture_baker.h"
#include "texture_baker/font.h"
#include "rect.h"

namespace augs {
	namespace graphics {
		namespace gui {
			namespace text {
				struct formatted_char {
					augs::font* font_used;
					wchar_t c;
					unsigned char r, g, b, a;
					void set(wchar_t, augs::font* = 0, const rgba& = rgba());
					void set(augs::font* = 0, const rgba& = rgba());
				};

				struct style {
					augs::font* f;
					rgba color;
					style(augs::font* = nullptr, rgba = rgba());
					style(const formatted_char&);
					operator formatted_char();
				};

				typedef std::basic_string<formatted_char> fstr;
			}

			class gui_world {
			public:
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
					rect* subject = nullptr;
					float speed_mult = 1.f;
				};

				static clipboard global_clipboard;

				rect* rect_in_focus;

				middlescroll_data middlescroll;

				augs::window::event::state state;

				float delta_milliseconds = 1000 / 60.f;

				rect *rect_held_by_lmb = nullptr;
				rect *rect_held_by_rmb = nullptr;

				rect root;
				std::vector<augs::vertex_triangle> triangle_buffer;

				gui_world();

				void set_delta_milliseconds(float);

				void set_focus(rect*);
				rect* get_rect_in_focus() const;

				void consume_raw_input_and_generate_gui_events(augs::window::event::state);
				void perform_logic_step();
				void draw_triangles();
			};
				
			
			namespace text {
				extern fstr format(const std::wstring&, style);
				extern void format(const std::wstring&, style, fstr&);
				extern void format(const wchar_t*, style, fstr&);
				extern fstr format(const wchar_t*, style);
			}

			extern void paste_clipboard(text::fstr& out, text::formatted_char = text::formatted_char());
		}
	}
	namespace misc {
		extern std::wstring wstr(const graphics::gui::text::fstr& f);
		template <class T>
		T wnum(const graphics::gui::text::fstr& f) {
			std::wistringstream ss(wstr(f));
			T v;
			ss >> v;
			return v;
		}
		
	}
}

