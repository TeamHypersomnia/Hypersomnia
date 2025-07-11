#include <cstddef>
#include <algorithm>
#include "augs/gui/text/ui.h"
#include "augs/window_framework/window.h"

#undef min
#undef max
namespace augs {
	namespace gui {
		namespace text {
			ui::ui(style default_style) :
				caret(default_style), anchor_pos(0),
				bold_bound(false),
				italics_bound(false),
				forced_bold(false),
				redraw(true),
				forced_italics(false), max_characters(0), whitelist(nullptr), blacklist(nullptr), allow_unknown_characters_as_default(false) {}
			const augs::baked_font& ui::getf(unsigned i) const {
				return get_all_assets()
					[(i < get_str().length() && get_str()[i].format.font != assets::font_id::INVALID) ? get_str()[i].format.font : caret.default_style.font];
			}

			void ui::anchor() {
				anchor_pos = caret.pos;
			}

			void ui::clean_selection() {
				str().erase(get_str().begin() + get_left_selection(), get_str().begin() + get_right_selection());
				caret.pos = get_left_selection();
				caret.selection_offset = 0;
			}


			int ui::get_selection_offset() {
				return caret.selection_offset;
			}

			void ui::set_selection_offset(int new_offset) {
				caret.selection_offset = new_offset;
			}

			unsigned ui::get_caret_pos() const {
				return caret.pos;
			}

			void ui::set_caret_end(bool select) {
				set_caret(get_str().length(), select);
			}

			void ui::set_caret(unsigned pos, bool s) {
				//if(!s) select.type = select.LETTERS;
				//selection temp = select; 
				if (caret.pos <= pos) {
					caret_right(pos - caret.pos, s);
					//if(select.type == select.WORDS) {
					//	// caret_right_word(true);
					//}
				}
				else if (caret.pos > pos) {
					caret_left(caret.pos - pos, s);
					//if(select.type == select.WORDS) {
					//	///caret_left_word(true);
					//}

				}
				else return;

				//if(!caret.selection_offset) {
				//	//	caret_left_word(true);
				////}
				//if(false && select.type == select.WORDS) {
				//	if(!caret.selection_offset) {
				//		select = temp;
				//		caret.selection_offset = -temp.offset;
				//	}
				//	else if(caret.selection_offset > 0){
				//		caret_left_word(true);
				//	}
				//	else if(caret.selection_offset < 0)
				//		caret_right_word(true);
				//}


			}

			void ui::caret_left(bool s) {
				if (caret.selection_offset && !s) {
					if (caret.selection_offset < 0)
						caret_left(-caret.selection_offset, false);
					caret.selection_offset = 0;
				}
				else if (caret.pos > 0) {
					--caret.pos;
					if (s) ++caret.selection_offset;

					//if(separator.is_character_newline(get_str()[caret.pos].c)) --caret.line;
					anchor();
				}
				unbind_styles();
			}

			void ui::caret_right(bool s) {
				if (caret.selection_offset && !s) {
					if (caret.selection_offset > 0)
						caret_right(caret.selection_offset, false);
					caret.selection_offset = 0;
				}
				else if (caret.pos < get_str().length()) {
					++caret.pos;
					if (s) --caret.selection_offset;

					//if(separator.is_character_newline(get_str()[caret.pos-1].c)) ++caret.line;
					anchor();
				}
				unbind_styles();
			}

			void ui::caret_left(unsigned n, bool s) {
				caret.pos -= (n = std::min(caret.pos, n));
				anchor();
				if (s) caret.selection_offset += n;
				else caret.selection_offset = 0;
				unbind_styles();
			}

			void ui::caret_right(unsigned n, bool s) {
				caret.pos += (n = std::min<unsigned long>(n, get_str().length() - caret.pos));
				anchor();
				if (s) caret.selection_offset -= n;
				else caret.selection_offset = 0;
				unbind_styles();
			}

			void ui::caret_left_word(bool s) {
				caret_left(separator.get_left_word(get_str(), caret.pos), s);
			}

			void ui::caret_right_word(bool s) {
				caret_right(separator.get_right_word(get_str(), caret.pos), s);
			}

			void ui::remove_line() {
				select_line(caret.pos);
				if (caret.selection_offset) {
					backspace();
				}
			}

			void ui::cut(clipboard& sys) {
				if (caret.selection_offset) {
					copy(sys);
					backspace();
				}
				else {
					select_line(caret.pos);
					if (caret.selection_offset) {
						copy(sys);
						backspace();
					}
				}
			}

			void ui::copy(clipboard& sys) {
				if (caret.selection_offset)
					sys.copy_clipboard(get_str().substr(get_left_selection(), std::abs(caret.selection_offset)));
				else {
					unsigned tmp = caret.pos;
					select_line(caret.pos);
					if (caret.selection_offset) sys.copy_clipboard(get_str().substr(get_left_selection(), std::abs(caret.selection_offset)));
				}

			}

			void ui::paste(clipboard& clip) {
				if (clip.is_clipboard_own())
					insert(clip.contents);
				else {
					formatted_string out;
					paste_clipboard_formatted(out, get_current_style());
					insert(out);
				}
			}

			void ui::insert(formatted_string& ss) {
				formatted_string s;

				for (size_t i = 0; i < ss.length(); ++i)
					if (is_valid_glyph(ss[i]) && is_whitelisted(ss[i].utf_unit) && !is_blacklisted(ss[i].utf_unit)) s += ss[i];

				if (max_characters > 0) {
					size_t newlen = get_str().size() + s.size() - std::abs(caret.selection_offset);
					if (newlen > max_characters) {
						s.erase(s.size() - (newlen - max_characters));
					}
				}

				if (caret.selection_offset) {
					edit.action(action(*this, get_left_selection(), s, get_str().substr(get_left_selection(), std::abs(caret.selection_offset))));
					clean_selection();
				}
				else edit.action(action(*this, caret.pos, s, action::INSERT));

				str().insert(caret.pos, s);
				caret.pos += s.length();
				edit.front().set_redo();

				anchor();
				set_needs_redraw();
			}

			void ui::character(const char& cc) {
				formatted_char ch = get_current_style();
				ch.utf_unit = cc;
				character(ch);
				unbind_styles();
				set_needs_redraw();
			}

			void ui::character(const formatted_char& ch) {
				if (!is_valid_glyph(ch) || !is_whitelisted(ch.utf_unit) || is_blacklisted(ch.utf_unit) || (max_characters > 0 && get_str().size() + 1 - std::abs(caret.selection_offset) > max_characters)) return;

				if (caret.selection_offset) {
					edit.action(action(*this, get_left_selection(), ch, get_str().substr(get_left_selection(), get_right_selection() - get_left_selection())));
					clean_selection();
				}
				else edit.action(action(*this, caret.pos, ch));
				str().insert(get_str().begin() + caret.pos, 1, ch);
				++caret.pos;
				edit.front().set_redo();
				anchor();
			}

			void ui::backspace(bool ctrl) {
				if (caret.selection_offset) {
					edit.action(action(*this, get_left_selection(), get_str().substr(get_left_selection(), get_right_selection() - get_left_selection()), action::ERASE));
					clean_selection();
					edit.front().set_redo();
					set_needs_redraw();
				}
				else if (ctrl) {
					unsigned left_offset = separator.get_left_word(get_str(), caret.pos);
					if (caret.pos > left_offset - 1) {
						edit.action(action(*this, caret.pos - left_offset, get_str().substr(caret.pos - left_offset, left_offset), action::ERASE));
						caret.pos -= left_offset;
						str().erase(get_str().begin() + caret.pos, get_str().begin() + caret.pos + left_offset);
						edit.front().set_redo();
						set_needs_redraw();
					}
				}
				else if (caret.pos > 0) {
					edit.action(action(*this, caret.pos - 1, get_str().substr(caret.pos - 1, 1), action::ERASE));
					str().erase(get_str().begin() + --caret.pos);
					edit.front().set_redo();
					set_needs_redraw();
				}
				anchor();
			}

			void ui::del(bool ctrl) {
				if (caret.selection_offset) {
					edit.action(action(*this, get_left_selection(), get_str().substr(get_left_selection(), get_right_selection() - get_left_selection()), action::ERASE));
					clean_selection();
					edit.front().set_redo();
					//anchor();
				}
				else if (ctrl) {
					int right_offset = separator.get_right_word(get_str(), caret.pos);
					if (caret.pos + right_offset - 1 < get_str().length()) {
						edit.action(action(*this, caret.pos, get_str().substr(caret.pos, right_offset), action::ERASE));
						str().erase(get_str().begin() + caret.pos, get_str().begin() + caret.pos + right_offset);
						edit.front().set_redo();
					}
				}
				else if (caret.pos < get_str().length()) {
					edit.action(action(*this, caret.pos, get_str().substr(caret.pos, 1), action::ERASE));
					str().erase(get_str().begin() + caret.pos);
					edit.front().set_redo();
				}
				anchor();
				set_needs_redraw();
			}

			void ui::select_all() {
				caret.pos = get_str().length();
				caret.selection_offset = -int(caret.pos);
				anchor();
			}

			void ui::select_word(unsigned at) {
				if (get_str().empty()) return;

				int left = 0, right = 0;
				auto chr = get_str()[std::min<unsigned long>(at, get_str().length() - 1)].utf_unit;

				if (at >= get_str().length() || separator.is_character_newline(chr))
					left = separator.get_left_word(get_str(), at);
				else {
					int type = separator.word_type(chr);
					left = separator.get_left_word(get_str(), at, 0, type);
					right = separator.get_right_word(get_str(), at, get_str().length(), type);
				}

				caret.pos = std::min<unsigned long>(at + right, get_str().length());
				caret.selection_offset = -int(right + left);
				anchor();
			}

			void ui::select_line(unsigned at) {
				if (get_str().empty()) return;
				auto& line = get_draft().lines[get_draft().get_line(at)];
				caret.pos = line.begin;
				caret.selection_offset = int(line.end - line.begin);
				anchor();
			}

			void ui::bold() {
				if (caret.selection_offset) {
					bool bold_all = false;
					int l = get_left_selection(), r = get_right_selection();
					edit.action(action(*this, l, r, 0, std::vector<bool>(), action::BOLDEN));
					for (int i = l; i < r; ++i) {
						if (getf(i).can_be_bolded() && !getf(i).is_bolded()) {
							bold_all = true;
							break;
						}
					}
					edit.front().unapply = !bold_all;
					edit.front().set_redo();

					for (int i = l; i < r; ++i) {
						auto& f = getf(i);
						edit.front().states.push_back(f.is_bolded());
						str()[i].format.font = f.get_bold(bold_all);
					}
				}
				else if (forced_bold = !forced_bold)
					bold_bound = !get_all_assets()[get_neighbor_style().font].is_bolded();
				set_needs_redraw();
			}

			void ui::italics() {
				if (caret.selection_offset) {
					bool it_all = false;
					int l = get_left_selection(), r = get_right_selection();
					edit.action(action(*this, l, r, 0, std::vector<bool>(), action::ITALICSEN));
					for (int i = l; i < r; ++i) {
						if (getf(i).can_be_italicsed() && !getf(i).is_italicsed()) {
							it_all = true;
							break;
						}
					}
					edit.front().unapply = !it_all;
					edit.front().set_redo();

					for (int i = l; i < r; ++i) {
						auto& f = getf(i);
						edit.front().states.push_back(f.is_italicsed());
						str()[i].format.font = f.get_italics(it_all);
					}
				}
				else if (forced_italics = !forced_italics)
					italics_bound = !get_all_assets()[get_neighbor_style().font].is_italicsed();
			}

			bool ui::undo() {
				set_needs_redraw();
				return edit.undo();
			}

			bool ui::redo() {
				set_needs_redraw();
				return edit.redo();
			}

			void ui::caret_up(bool s) {
				guarded_redraw();
				if (!s) {
					caret.pos = get_left_selection();
				}

				int line = get_draft().get_line(caret.pos);
				if (line > 0) {
					auto c = get_draft().lines[line - 1].hover(get_draft().sectors[std::min<unsigned long>(get_draft().sectors.size() - 1, anchor_pos)], get_draft().sectors);
					caret.selection_offset += caret.pos - c;
					caret.pos = c;
				}

				if (!s) caret.selection_offset = 0;
			}

			void ui::caret_down(bool s) {
				guarded_redraw();
				if (!s) {
					caret.pos = get_right_selection();
				}

				unsigned line = get_draft().get_line(caret.pos);
				if (line < get_draft().lines.size() - 1) {
					auto c = get_draft().lines[line + 1].hover(get_draft().sectors[std::min<unsigned long>(get_draft().sectors.size() - 1, anchor_pos)], get_draft().sectors);
					caret.selection_offset -= c - caret.pos;
					caret.pos = c;
				}

				if (!s) caret.selection_offset = 0;
			}

			void ui::home(bool s) {
				guarded_redraw();
				set_caret(get_draft().lines[get_draft().get_line(caret.pos)].begin, s);
			}

			void ui::end(bool s) {
				guarded_redraw();
				set_caret(get_draft().lines[get_draft().get_line(caret.pos)].end, s);
			}

			ui::action::action(ui& subject, int where, const formatted_char& ch)
				: subject(&subject), where(where), character(ch), flag(CHARACTERS) {
				set_undo();
			}

			ui::action::action(ui& subject, int where, const formatted_char& ch, const formatted_string& replaced)
				: subject(&subject), where(where), character(ch), replaced(replaced), flag(REPLACE_CHARACTERS) {
				set_undo();
			}

			ui::action::action(ui& subject, int where, const formatted_string& _str, type flag)
				: subject(&subject), where(where), _str(_str), flag(flag) {
				set_undo();
			}

			ui::action::action(ui& subject, int where, const formatted_string& _str, const formatted_string& replaced)
				: subject(&subject), where(where), _str(_str), replaced(replaced), flag(action::REPLACE) {
				set_undo();
			}

			ui::action::action(ui& subject, int where, int right, bool unapply, const std::vector<bool>& v, type flag)
				: subject(&subject), where(where), right(right), unapply(unapply), states(v), flag(flag) {
				set_undo();
			}

			bool ui::action::include(const action& next) {
				if ((flag == CHARACTERS || flag == REPLACE_CHARACTERS) && next.flag == CHARACTERS && !subject->separator.is_character_newline(next.character.utf_unit)
					&& next.where == where + _str.length() + 1 /* we don't want to merge characters at different positions */
					) {
					_str += next.character;
					return true;
				}
				return false;
			}

			void ui::action::execute(bool undo) {
				if (flag == action::NONE) return;

				if (flag == (undo ? action::INSERT : action::ERASE)) {
					subject->str().erase(where, _str.length());
				}
				else if (flag == (undo ? action::ERASE : action::INSERT)) {
					subject->str().insert(where, _str);
				}

				if (flag == action::CHARACTERS) {
					if (undo) {
						subject->str().erase(where, _str.length() + 1);
					}
					else {
						subject->str().insert(where, 1, character);
						subject->str().insert(where + 1, _str);
					}
				}

				if (flag == action::REPLACE) {
					subject->str().erase(where, undo ? _str.length() : replaced.length());
					subject->str().insert(where, undo ? replaced : _str);
				}

				if (flag == action::REPLACE_CHARACTERS) {
					subject->str().erase(where, undo ? _str.length() + 1 : replaced.length());
					if (undo)
						subject->str().insert(where, replaced);
					else {
						subject->str().insert(where, 1, character);
						subject->str().insert(where + 1, _str);
					}
				}

				subject->caret.pos = undo ? un.caret_pos : re.caret_pos;
				subject->caret.selection_offset = undo ? un.sel_offset : re.sel_offset;
				subject->anchor();

				if (flag == action::BOLDEN || flag == action::ITALICSEN) {
					for (int i = where; i < right; ++i) {
						assets::font_id& f = subject->str()[i].format.font;
						f = flag == action::BOLDEN ? subject->getf(i).get_bold(undo ? (unapply ? true : states[i - where]) : !unapply) :
							subject->getf(i).get_italics(undo ? (unapply ? true : states[i - where]) : !unapply);
					}
				}
			}

			void ui::action::set_undo() {
				un.caret_pos = subject->caret.pos;
				un.sel_offset = subject->caret.selection_offset;
			}

			void ui::action::set_redo() {
				re.caret_pos = subject->caret.pos;
				re.sel_offset = subject->caret.selection_offset;
			}

			unsigned ui::get_left_selection() const {
				return caret.get_left_selection();
			}

			unsigned ui::get_right_selection() const {
				return caret.get_right_selection();
			}

			void ui::unbind_styles() {
				forced_bold = forced_italics = false;
			}

			style ui::get_current_style() const {
				style ch = get_neighbor_style();
				if (forced_bold)    ch.font = get_all_assets()[ch.font].get_bold(bold_bound);
				if (forced_italics) ch.font = get_all_assets()[ch.font].get_italics(italics_bound);
				return ch;
			}

			style ui::get_default_style() const {
				return caret.default_style;
			}

			style ui::get_neighbor_style() const {
				if (!get_str().empty()) return (caret.pos ? get_str()[caret.pos - 1] : get_str()[0]);
				else return caret.default_style;
			}

			bool ui::get_bold_status() const {
				if (caret.selection_offset) {
					int l = get_left_selection(), r = get_right_selection();
					for (int i = l; i < r; ++i)
						if (!getf(i).is_bolded())
							return false;
					return true;
				}
				else return forced_bold ? bold_bound : get_all_assets()[get_neighbor_style().font].is_bolded();
			}

			bool ui::get_italics_status() const {
				if (caret.selection_offset) {
					int l = get_left_selection(), r = get_right_selection();
					for (int i = l; i < r; ++i)
						if (!getf(i).is_italicsed())
							return false;
					return true;
				}
				else return forced_italics ? italics_bound : get_all_assets()[get_neighbor_style().font].is_italicsed();
			}

			bool ui::is_valid_glyph(const formatted_char& c) {
				return
					/* if we want to have newlines, we need a whitespace glyph added to the end of every line
						similiarly with tabs, we want to replace them with spaces
					*/
					(iswspace(c.utf_unit) && get_all_assets()[c.format.font].meta_from_file.find_glyph(' ') != nullptr)
					|| allow_unknown_characters_as_default || get_all_assets()[c.format.font].meta_from_file.find_glyph(c.utf_unit) != nullptr;
			}

			bool ui::is_whitelisted(char c) const {
				if (!whitelist) return true;
				const char* it = whitelist;
				while (*it) if (*it++ == c) return true;
				return false;
			}

			bool ui::is_blacklisted(char c) const {
				if (!blacklist) return false;
				const char* it = blacklist;
				while (*it) if (*it++ == c) return true;
				return false;
			}
		}
	}
}
