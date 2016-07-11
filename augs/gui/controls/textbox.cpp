#pragma once
#include "textbox.h"
#include "gui/text/drafter.h"
#include "templates.h"
#include "window_framework/platform_utils.h"

namespace augs {
	namespace gui {
		namespace controls {
			textbox::textbox(const rect& r, text::style default_style)
				: rect(r), editor(default_style), view_caret(false), blink_reset(false) {
				//preserve_focus = true; 
			}

			void textbox::show_caret() {
				editor.guarded_redraw();
				scroll += editor.get_draft().view_caret(editor.get_caret_pos(), get_local_clipper());
				view_caret = false;
			}

			void textbox::draw_text_ui(draw_info in) {
				editor.guarded_redraw();
				print.draw_text(in.v, editor, *this);
			}

			void textbox::perform_logic_step(rect_world& owner) {
				drag.move(scroll, owner.delta_milliseconds());

				if (drag.vel[0] != 0.f || drag.vel[1] != 0.f)
					on_drag(owner.state.mouse.pos);

				print.blink.update();
			}

			void textbox::draw_triangles(draw_info in) {
				draw_text_ui(in);
			}

			vec2i textbox::local_mouse(vec2i global_mouse) {
				return global_mouse + scroll - get_absolute_xy();
			}

			void textbox::on_caret_left(bool s) { editor.caret_left(s);			view_caret = true; blink_reset = true; }
			void textbox::on_caret_right(bool s) { editor.caret_right(s);		view_caret = true; blink_reset = true; }
			void textbox::on_caret_left_word(bool s) { editor.caret_left_word(s);	view_caret = true; blink_reset = true; }
			void textbox::on_caret_right_word(bool s) { editor.caret_right_word(s);	view_caret = true; blink_reset = true; }
			void textbox::on_caret_up(bool s) { editor.caret_up(s);			view_caret = true; blink_reset = true; }
			void textbox::on_caret_down(bool s) { editor.caret_down(s);			view_caret = true; blink_reset = true; }
			void textbox::on_caret_ctrl_up() {
				editor.guarded_redraw();
				int li = editor.get_draft().get_line_visibility(get_local_clipper()).first;
				if (li == -1) return;
				auto& l = editor.get_draft().lines[li];

				if (int(scroll.y) != l.get_rect().y)
					scroll.y = float(l.get_rect().y);
				else if (li > 0)
					scroll.y -= float(editor.get_draft().lines[li - 1].get_rect().h);
			}

			void textbox::on_caret_ctrl_down() {
				editor.guarded_redraw();
				int li = editor.get_draft().get_line_visibility(get_local_clipper()).second;
				if (li == -1) return;
				auto& l = editor.get_draft().lines[li];

				if (int(scroll.y) != l.get_rect().b() - rc.h())
					scroll.y = float(l.get_rect().b() - rc.h());
				else if (li < int(editor.get_draft().lines.size() - 1))
					scroll.y += float(editor.get_draft().lines[li + 1].get_rect().h);
			}

			void textbox::on_place_caret(vec2i mouse, bool s) {
				editor.guarded_redraw();
				//if(!s) caret.selection_offset = 0;
				editor.set_caret(editor.get_draft().map_to_caret_pos(local_mouse(mouse)), s);
				blink_reset = true;
				if (!s) view_caret = true;
			}

			void textbox::on_select_word(vec2i mouse) {
				editor.guarded_redraw();
				editor.select_word(editor.get_draft().map_to_caret_pos(local_mouse(mouse)));
				blink_reset = true;
				view_caret = true;
			}

			void textbox::on_select_line(vec2i mouse) {
				editor.guarded_redraw();
				editor.select_line(editor.get_draft().map_to_caret_pos(local_mouse(mouse)));
				blink_reset = true;
				view_caret = true;
			}

			void textbox::on_select_all() { editor.select_all();								view_caret = true; }
			void textbox::on_home(bool s) { editor.home(s);									view_caret = true; }
			void textbox::on_end(bool s) { editor.end(s);									view_caret = true; }
			void textbox::on_pagedown() { scroll.y += get_rect_absolute().h(); }
			void textbox::on_pageup() { scroll.y -= get_rect_absolute().h(); }
			void textbox::on_character(wchar_t c) { editor.character(c);								view_caret = true; blink_reset = true; }
			void textbox::on_cut(rect_world& sys) { editor.cut(sys.global_clipboard);					view_caret = true; }
			void textbox::on_bold() { editor.bold();									view_caret = true; }
			void textbox::on_italics() { editor.italics();									view_caret = true; }
			void textbox::on_copy(rect_world& sys) { editor.copy(sys.global_clipboard);				view_caret = true; }
			void textbox::on_undo() { editor.undo();									view_caret = true; }
			void textbox::on_redo() { editor.redo();									view_caret = true; }
			void textbox::on_paste(rect_world& sys) { editor.paste(sys.global_clipboard);				view_caret = true; blink_reset = true; }
			void textbox::on_backspace(bool c) { editor.backspace(c);								view_caret = true; blink_reset = true; }
			void textbox::on_del(bool c) { editor.del(c);									view_caret = true; blink_reset = true; }

			void textbox::on_drag(vec2i mouse) {
				editor.set_caret(editor.get_draft().map_to_caret_pos(local_mouse(mouse)), true);
				drag.drag(local_mouse(mouse), get_local_clipper());
				blink_reset = true;
			}

			rects::wh<float> textbox::get_content_size() {
				editor.guarded_redraw();
				return editor.get_draft().get_bbox();
			}

			void textbox::pass_gui_event_to_text_editor(event_info e) {
				using namespace augs::window::event::keys;
				auto& w = e.owner.state;
				auto& k = w.keys;
				vec2i mouse = w.mouse.pos;
				bool s = k[SHIFT], c = k[CTRL];

				switch (e.msg) {
				case rect::gui_event::keydown:
					switch (w.key) {
					case LEFT:
						if (c) on_caret_left_word(s);
						else  on_caret_left(s);
						break;
					case RIGHT:
						if (c) on_caret_right_word(s);
						else  on_caret_right(s);
						break;
					case UP:
						if (c) on_caret_ctrl_up();
						else  on_caret_up(s);
						break;
					case DOWN:
						if (c) on_caret_ctrl_down();
						else  on_caret_down(s);
						break;
					case BACKSPACE: if (editable) on_backspace(c); break;
					case DEL: if (editable) on_del(c); break;
					case HOME: on_home(s); break;
					case END: on_end(s); break;
					case PAGEUP: on_pageup(); break;
					case PAGEDOWN: on_pagedown(); break;
					default:
						if (c && !k[RALT])
							switch (w.key) {
							case A: on_select_all(); break;
							case B: if (editable) on_bold();		 break;
							case I: if (editable) on_italics();	 break;
							case C: on_copy(e.owner);		 break;
							case Z: if (editable) k[LSHIFT] ? on_redo() : on_undo(); break;
							case Y: if (editable) on_redo();		 break;
							case X: if (editable) on_cut(e.owner);		 break;
							case V: if (editable) on_paste(e.owner);		 break;
							default: break;
							}
						break;
					}
					break;

				case rect::gui_event::character:
					if (editable && w.key != BACKSPACE && !c) on_character(w.utf16);

					break;

				case rect::gui_event::ldown: on_place_caret(mouse, false); break;
				case rect::gui_event::rdown: break;
				case rect::gui_event::lpressed: on_place_caret(mouse, true); break;
				case rect::gui_event::loutdrag: on_drag(mouse); break;
				case rect::gui_event::ldoubleclick: on_select_word(mouse); break;
				case rect::gui_event::ltripleclick: on_select_line(mouse); break;

				case rect::gui_event::lup:		drag.stop(); break;
				case rect::gui_event::hover:	drag.stop(); break;
				case rect::gui_event::loutup:	drag.stop(); break;

				case rect::gui_event::focus: print.active = true;   blink_reset = true; break;
				case rect::gui_event::blur: print.active = false;  break;

				default: break;
				}

				if (view_caret) {
					show_caret();
				}

				if (blink_reset) {
					print.blink.reset();
					blink_reset = false;
				}
			}

			void textbox::consume_gui_event(event_info e) {
				pass_gui_event_to_text_editor(e);
				try_to_enable_middlescrolling(e);
				try_to_make_this_rect_focused(e);
				scroll_content_with_wheel(e);
			}


			property_textbox::property_textbox(vec2i pos, int width, text::style default_style, std::function<void(std::wstring&)> property_guard)
				: textbox(rects::xywh<float>(pos.x, pos.y, width, (*default_style.f).get_height()), default_style), property_guard(property_guard) {
			}

			std::wstring property_textbox::get_str() const {
				return formatted_string_to_wstring(editor.get_str());
			}

			void property_textbox::consume_gui_event(event_info e) {
				if (e.msg == rect::gui_event::blur) {
					std::wstring ws = formatted_string_to_wstring(editor.get_str());
					if (property_guard) property_guard(ws);
					editor.select_all();
					editor.insert(text::format(ws, editor.get_default_style()));
				}


				/* if e belongs neither to tab nor enter, handle interface */
				if (!focus_next_rect_by_tab(e) && !focus_next_rect_by_enter(e))
					pass_gui_event_to_text_editor(e);

				try_to_enable_middlescrolling(e);
				try_to_make_this_rect_focused(e);
				scroll_content_with_wheel(e);
			}

			void property_textbox::on_character(wchar_t c) {
				if (!augs::window::is_character_newline(c)) textbox::on_character(c);
			}
		}
	}
}