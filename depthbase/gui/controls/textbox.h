#pragma once
#include "../dragger.h"
#include "../rect.h"
#include "../text\printer.h"
#include "../text\ui.h"
#include "misc/undoredo.h"
#include <functional>

namespace augs {
	namespace graphics {
		namespace gui {
			namespace controls {
				class textbox : public rect {
					vec2<int> local_mouse(vec2<int> global_mouse);
				public:
					virtual void on_caret_left(bool select);
					virtual void on_caret_right(bool select);
					virtual void on_caret_left_word(bool select);
					virtual void on_caret_right_word(bool select);
					virtual void on_caret_up(bool select);
					virtual void on_caret_down(bool select);
					virtual void on_caret_ctrl_up();
					virtual void on_caret_ctrl_down();
					virtual void on_place_caret(vec2<int> mouse, bool select);
					virtual void on_select_word(vec2<int> mouse);
					virtual void on_select_line(vec2<int> mouse);
					virtual void on_select_all();
					virtual void on_home(bool select);
					virtual void on_end(bool select);
					virtual void on_pagedown();
					virtual void on_pageup();
					virtual void on_character(wchar_t);
					virtual void on_cut(system&);
					virtual void on_bold();
					virtual void on_italics();
					virtual void on_copy(system&);
					virtual void on_paste(system&);
					virtual void on_undo();
					virtual void on_redo();
					virtual void on_backspace(bool);
					virtual void on_del(bool);
					virtual void on_drag(vec2<int> mouse);

					virtual rects::wh<float> get_content_size() override;
					virtual void event_proc(event_info) override;
					virtual void update_proc(group&) override;

					void draw_text_ui(draw_info);
					void handle_interface(event_info);

					bool view_caret, blink_reset, editable = true;
					dragger drag;
					
					text::ui editor;
					text::printer print;

					void show_caret();
					textbox(const rect& = rect(), text::style default_style = text::style());
				};

				/* one-line textbox */
				struct property_textbox : public textbox {
					virtual void event_proc(event_info) override;
					
					/* we should not pass newlines through */
					virtual void on_character(wchar_t) override;

					std::function<void (std::wstring&)> property_guard;

					property_textbox(vec2<int> pos, int width, text::style default_style, std::function<void (wstring&)> property_guard = nullptr);
					
					template <class T>
					void set(T val) {
						auto s = misc::wstr(val);
						if(property_guard) property_guard(s);

						editor.select_all();
						editor.insert(format(s, editor.get_default_style()));
					}

					template <>
					void set(std::wstring s) {
						if(property_guard) property_guard(s);

						editor.select_all();
						editor.insert(format(s, editor.get_default_style()));
					}
					
					template <class T>
					T get() const {
						return wnum<T>(editor.get_str());
					}

					std::wstring get_str() const;

				};
			}
		}
	}
}