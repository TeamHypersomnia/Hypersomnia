#include "math/vec2.h"
#include "hypersomnia_gui.h"

using namespace stylesheeted;
callback_textbox::callback_textbox(gui_group& owner) : owner(&owner) {
	owner.main_window.root.children.push_back(&myscrtx);
	owner.main_window.root.children.push_back(&myscrhtx);
	owner.main_window.root.children.push_back(&textbox_object);
}

void callback_textbox::clear_text() {
	textbox_object.editor.select_all();
	textbox_object.editor.backspace();
}

bool callback_textbox::is_clean() {
	return textbox_object.editor.get_str().empty();
}

void callback_textbox::set_area(augs::rects::xywh<float> area) {
	textbox_object.rc = rects::xywh<float>(area.x, area.y, area.w, area.h);
	textbox_object.editor.draft().wrap_width = static_cast<unsigned>(area.w);

}

void callback_textbox::set_wrapping_width(unsigned w) {
	textbox_object.editor.draft().wrap_width = w;
}

void callback_textbox::setup(augs::rects::xywh<float> area, bool is_input_textbox, font* default_font)
{
	//background = crect(rect_xywh(0, 0, 1000, 1000));
	textbox_object = textbox_wrapper(ctextbox(textbox(area, augs::graphics::gui::text::style(default_font, augs::rgba(255, 255, 255, 255)))));
	textbox_object.is_input_textbox = is_input_textbox;
	textbox_object.sl = &myscrtx;
	textbox_object.slh = &myscrhtx;
	myscrtx.enabled = false;
	myscrhtx.enabled = false;
	//background.scrollable = false;
	//background.clip = false;

	//chat_content = ctextbox(textbox(rect_xywh(rect_ltrb(20, 20, 600 - 20, 400 - 20)), text::style(fonts + 0, white)));

	/* structural settings */
	textbox_object.editor.draft().wrap_width = 0;
	textbox_object.editor.draft().kerning = false;

	textbox_object.editor.draft().wrap_width = area.w;
	//chat_textbox.rc.b = background.rc.b;

	/* visual settings */
	textbox_object.print.blink.blink = true;
	textbox_object.print.blink.interval_ms = 500;
	textbox_object.print.selection_bg_mat = rgba(128, 255, 255, 120);
	textbox_object.print.selection_inactive_bg_mat = rgba(128, 255, 255, 40);
	textbox_object.print.highlight_mat = rgba(15, 15, 15, 150);
	textbox_object.print.caret_mat = rgba(255, 255, 255, 255);
	textbox_object.print.highlight_current_line = true;
	textbox_object.print.highlight_during_selection = true;
	textbox_object.print.align_caret_height = true;
	textbox_object.print.caret_width = 1;
	textbox_object.drag.vel_mult = 100.0;
	textbox_object.editable = is_input_textbox;

	myscrhtx.align();
	myscrtx.align();
}


callback_rect::callback_rect(gui_group& owner) : owner(&owner) {
	owner.main_window.root.children.push_back(&rect_obj);
}

void callback_rect::setup(augs::rects::xywh<float> area, bool focusable) {
	rect_obj = rect_wrapper(crect(area));
	rect_obj.focusable = focusable;
}

void callback_rect::focus() {
	owner->main_window.set_focus(&rect_obj);
}

bool callback_textbox::is_focused() {
	return owner->main_window.get_rect_in_focus() == &textbox_object;
}

void callback_textbox::set_caret(unsigned pos, bool select) {
	textbox_object.editor.set_caret(pos, select);
}

void callback_textbox::set_alpha_range(int pos1, int pos2, unsigned val) {
	for (int i = pos1; i < pos2; ++i) {
		textbox_object.editor.str()[i].a = val;
	}
}

void callback_textbox::remove_line() {
	textbox_object.editor.remove_line();
}

void callback_textbox::backspace() {
	textbox_object.editor.backspace();
}

void callback_textbox::view_caret() {
	textbox_object.show_caret();
}

vec2 callback_textbox::get_text_bbox() {
	auto result = textbox_object.editor.get_draft().get_bbox();
	return vec2(result.w, result.h);
}


void callback_textbox::draw(bool flag) {
	textbox_object.enable_drawing = flag;
}

void callback_textbox::focus() {
	owner->main_window.set_focus(&textbox_object);
}

unsigned callback_textbox::get_length() {
	return textbox_object.editor.get_str().size();
}

void gui_group::blur() {
	main_window.set_focus(nullptr);
}
