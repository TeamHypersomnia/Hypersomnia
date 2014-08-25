#include "stdafx.h"
#pragma once
#include "hypersomnia_gui.h"
#include "window_framework/window.h"

using namespace stylesheeted;

hypersomnia_gui::hypersomnia_gui(augs::window::glwindow& actual_window) : actual_window(actual_window) {
	images[0].create(4, 4, 1);
	images[0].fill(255);
	gui::null_texture = textures + 0;

	font_in fin;

	fin.init();
	wchar_t* str = L" qvxQVXaπbcÊdeÍfghijkl≥mnÒoÛprsútuwyzüøA•BC∆DE FGHIJKL£MN—O”PRSåTUWYZèØ0123456789.!@#$%^&*()_+-=[];'\\,./{}:\"|<>?";

	fontf[0].open(fin, "hypersomnia/data/visitor1.ttf", 10, str);

	fin.deinit();

	atl.quick_add(images, textures, IMAGES, fontf, fonts, FONTS);
	atl.pack(16000);
	atl.create_image(4, false);
	atl.build(false, false);
	atl.img.destroy();
	atl.nearest();

	gui::null_texture->translate_uv(pointf(2, 2));
	gui::null_texture->scale_uv(0.000000001f, 0.000000001f);

	ltblue_theme();

	float w = 800;
	float h = 500;

	/* structural settings */
	chat_textbox.editor.draft().wrap_width = 0;
	chat_textbox.editor.draft().kerning = false;
	
	chat_textbox.editor.draft().wrap_width = w;
	chat_textbox.rc.b = background.rc.b;
	
	/* visual settings */
	chat_textbox.print.blink.blink = true;
	chat_textbox.print.blink.interval_ms = 500;
	chat_textbox.print.selection_bg_mat = pixel_32(128, 255, 255, 120);
	chat_textbox.print.selection_inactive_bg_mat = pixel_32(128, 255, 255, 40);
	chat_textbox.print.highlight_mat = pixel_32(15, 15, 15, 255);
	chat_textbox.print.caret_mat = pixel_32(255, 255, 255, 255);
	chat_textbox.print.highlight_current_line = true;
	chat_textbox.print.highlight_during_selection = true;
	chat_textbox.print.align_caret_height = true;
	chat_textbox.print.caret_width = 1;
	chat_textbox.drag.vel_mult = 100.0;

	chat_textbox.preserve_focus = true;

	myscrhtx.align();
	myscrtx.align();

	background.children.push_back(&myscrtx);
	background.children.push_back(&myscrhtx);
	background.children.push_back(&chat_textbox);

	main_window.root.children.push_back(&background);
	main_window.middlescroll.speed_mult = 90.0f;
	//main_window.middlescroll.mat = material(textures + 4, pixel_32(255, 255, 255, 180));
	main_window.set_focus(&chat_textbox);
} 

void hypersomnia_gui::poll_events() {
	memcpy(&gl.events, &actual_window.events, sizeof(actual_window.events));

	main_window.update_rectangles();
	main_window.poll_events();
}

void hypersomnia_gui::draw_call() {
	main_window.default_update();

	atl._bind();
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad) * main_window.quad_array.size(), main_window.quad_array.data(), GL_STREAM_DRAW);
	glDrawArrays(GL_QUADS, 0, main_window.quad_array.size() * 4);
}