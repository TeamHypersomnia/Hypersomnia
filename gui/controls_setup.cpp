#include "hypersomnia_gui.h"

using namespace stylesheeted;

void hypersomnia_gui::setup(augs::vec2<> camera) {
	camera_size = camera;
	images[0].create(4, 4, 1);
	images[0].fill(255);
	gui::null_texture = textures + 0;

	font_in fin;

	fin.init();
	wchar_t* str = L" qvxQVXaπbcÊdeÍfghijkl≥mnÒoÛprsútuwyzüøA•BC∆DE FGHIJKL£MN—O”PRSåTUWYZèØ0123456789.!@#$%^&*()_+-=[];'\\,./{}:\"|<>?";

	fontf[0].open(fin, "hypersomnia/data/Kubasta.ttf", 16, str);

	fin.deinit();

	atl.quick_add(images, textures, IMAGES, fontf, fonts, FONTS);
	atl.pack();
	atl.create_image(4, false);
	atl.build(false, false);
	atl.img.destroy();
	atl.nearest();

	gui::null_texture->translate_uv(pointf(2, 2));
	gui::null_texture->scale_uv(0.000000001f, 0.000000001f);

	ltblue_theme();

	main_window.middlescroll.speed_mult = 90.0f;
	focusable_bg = rect(rect_xywh(0, 0, camera_size.x, camera_size.y));

	focusable_bg.focusable = true;

	//main_window.root.children.push_back(&focusable_bg);

	//main_window.middlescroll.mat = material(textures + 4, pixel_32(255, 255, 255, 180));
}

rect_xywh _xywh(augs::rects::xywh<float>& xx) {
	return rect_xywh(xx.x, xx.y, xx.w, xx.h);
}

callback_textbox::callback_textbox(hypersomnia_gui& owner) : owner(&owner) {
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

void callback_textbox::setup(augs::rects::xywh<float> area, bool is_input_textbox)
{
	//background = crect(rect_xywh(0, 0, 1000, 1000));
	textbox_object = textbox_wrapper(ctextbox(textbox(_xywh(area), text::style(owner->fonts + 0, white))));
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

	//chat_textbox.editor.draft().wrap_width = w;
	//chat_textbox.rc.b = background.rc.b;

	/* visual settings */
	textbox_object.print.blink.blink = true;
	textbox_object.print.blink.interval_ms = 500;
	textbox_object.print.selection_bg_mat = pixel_32(128, 255, 255, 120);
	textbox_object.print.selection_inactive_bg_mat = pixel_32(128, 255, 255, 40);
	textbox_object.print.highlight_mat = pixel_32(15, 15, 15, 150);
	textbox_object.print.caret_mat = pixel_32(255, 255, 255, 255);
	textbox_object.print.highlight_current_line = true;
	textbox_object.print.highlight_during_selection = true;
	textbox_object.print.align_caret_height = true;
	textbox_object.print.caret_width = 1;
	textbox_object.drag.vel_mult = 100.0;
	textbox_object.editable = is_input_textbox;

	myscrhtx.align();
	myscrtx.align();
}


callback_rect::callback_rect(hypersomnia_gui& owner) : owner(&owner) {
	owner.main_window.root.children.push_back(&rect_obj);
}

void callback_rect::setup(augs::rects::xywh<float> area, bool focusable) {
	rect_obj = rect_wrapper(crect(_xywh(area)));
	rect_obj.focusable = focusable;
}

void callback_rect::focus() {
	owner->main_window.set_focus(&rect_obj);
}

bool callback_textbox::is_focused() {
	return owner->main_window.get_focus() == &textbox_object;
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

void callback_textbox::draw(bool flag) {
	textbox_object.draw = flag;
}

void callback_textbox::focus() {
	owner->main_window.set_focus(&textbox_object);
}

void hypersomnia_gui::blur() {
	main_window.set_focus(nullptr);
}
