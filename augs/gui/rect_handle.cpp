#include "rect_handle.h"
#include "rect.h"
#include "gui_event.h"

#include "stylesheet.h"
#include "augs/window_framework/window.h"

#include <algorithm>
#include "augs/ensure.h"
#include "rect_world.h"

#undef max
#undef min

using namespace augs::gui;

typedef augs::basic_pool<augs::gui::rect> B;
typedef rect R;

namespace augs {
	template <bool C>
	void basic_handle<C, B, R>::draw_stretched_texture(gui::draw_info in, const gui::material& mat) const {
		draw_clipped_rectangle(mat, this->get().get_rect_absolute(), get_parent().get(), in.v).good();
		// rc_clipped = draw_clipped_rectangle(mat, rc_clipped, parent, in.v);
	}

	template <bool C>
	void basic_handle<C, B, R>::draw_centered_texture(gui::draw_info in, const gui::material& mat, vec2i offset) const {
		auto absolute_centered = this->get().get_rect_absolute();
		auto tex_size = (*mat.tex).get_size();
		absolute_centered.l += absolute_centered.w() / 2 - float(tex_size.x) / 2;
		absolute_centered.t += absolute_centered.h() / 2 - float(tex_size.y) / 2;
		absolute_centered.l = int(absolute_centered.l) + offset.x;
		absolute_centered.t = int(absolute_centered.t) + offset.y;
		absolute_centered.w(tex_size.x);
		absolute_centered.h(tex_size.y);

		draw_clipped_rectangle(mat, absolute_centered, get_parent().get(), in.v).good();
		// rc_clipped = draw_clipped_rectangle(mat, rc_clipped, parent, in.v);
	}

	template <bool C>
	void basic_handle<C, B, R>::draw_rectangle_stylesheeted(gui::draw_info in, const gui::stylesheet& styles) const {
		auto st = styles.get_style();

		if (st.color.active || st.background_image.active)
			draw_stretched_texture(in, material(st));

		if (st.border.active) st.border.value.draw(in.v, *this);
	}

	template <bool C>
	basic_handle<C, B, R> basic_handle<C, B, R>::get_parent() const {
		return this->get_pool()[this->get().parent];
	}


	template <bool C>
	bool basic_handle<C, B, R>::is_being_dragged(gui::rect_world& g) const {
		return g.rect_held_by_lmb == *this && g.held_rect_is_dragged;
	}
	
	template <bool C>
	std::vector<basic_handle<C, B, R>> basic_handle<C, B, R>::get_children() const {
		return this->get_pool()[this->get().children];
	}
}

// explicit instantiation
template class augs::basic_handle<true, B, R>;
template class augs::basic_handle<false, B, R>;
