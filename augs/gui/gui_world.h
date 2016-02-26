#pragma once
#include <vector>
#include "math/vec2.h"
#include "window_framework/event.h"
#include "misc/timer.h"
#include "graphics/pixel.h"
#include "texture_baker/texture_baker.h"
#include "texture_baker/font.h"
#include "rect.h"

#include "game_framework/assets/font.h"
#include "misc/unsafe_type_collection.h"

namespace augs {
	namespace graphics {
		namespace gui {
			namespace text {
				struct formatted_char {
					assets::font_id font_used;
					wchar_t c;
					unsigned char r, g, b, a;
					void set(wchar_t, assets::font_id = assets::font_id::GUI_FONT, const rgba& = rgba());
					void set(assets::font_id = assets::font_id::GUI_FONT, const rgba& = rgba());
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
				fstr format(const std::wstring&, style);
				void format(const std::wstring&, style, fstr&);
				void format(const wchar_t*, style, fstr&);
				fstr format(const wchar_t*, style);
			}

			class gui_world {
				float delta_ms = 1000 / 60.f;
				// unsafe_container_collection<object_pool> controls_containers;

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
					rect_id subject = nullptr;
					float speed_mult = 1.f;
				};

				static clipboard global_clipboard;

				rect_id rect_in_focus;

				middlescroll_data middlescroll;

				augs::window::event::state state;

				rect *rect_held_by_lmb = nullptr;
				rect *rect_held_by_rmb = nullptr;

				rect root;
				std::vector<augs::vertex_triangle> triangle_buffer;

				gui_world();

				// template <class T>
				// void register_control_type() {
				// 	unsafe_type_collection::register_type<T>();
				// 	controls_containers.register_destructor<T>();
				// 	controls_containers.add<T>(1000);
				// }
				// 
				// template <class T, class... Args>
				// rect_id allocate(Args... args) {
				// 	auto typed_id = controls_containers.get<T>().allocate(args...);
				// 
				// 	rect_id result;
				// 	return *reinterpret_cast<rect_id*>(&typed_id);
				// }

				void set_delta_milliseconds(float);
				float delta_milliseconds();

				void set_focus(rect_id);
				rect_id get_rect_in_focus() const;

				void consume_raw_input_and_generate_gui_events(augs::window::event::state);
				void perform_logic_step();
				void draw_triangles();
			};
				
			void paste_clipboard_formatted(text::fstr& out, text::formatted_char = text::formatted_char());
		}
	}
}

