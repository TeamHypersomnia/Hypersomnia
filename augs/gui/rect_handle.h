#pragma once
#include <functional>
#include "misc/pool_handle.h"
#include "misc/pool.h"
#include "rect_id.h"

namespace augs {
	namespace gui {
		struct rect;
		class rect_world;
	}

	template <bool is_const>
	class basic_handle<is_const, basic_pool<gui::rect>, gui::rect> :
		public basic_handle_base<is_const, basic_pool<gui::rect>, gui::rect> {
	public:
		using basic_handle_base::basic_handle_base;
		using basic_handle_base::operator pool_id<gui::rect>;

		template <class = typename std::enable_if<!is_const>::type>
		operator gui::const_rect_handle() const;

		template <class = typename std::enable_if<!is_const>::type>
		void calculate_clipped_rectangle_layout(gui::content_size_behaviour) const;

		template <class = typename std::enable_if<!is_const>::type>
		void perform_logic_step(gui::rect_world&, gui::logic_behaviour callback) const;

		template <class = typename std::enable_if<!is_const>::type>
		void consume_gui_event(gui::event_info, gui::event_behaviour callback) const; /* event listener */

		void draw_triangles(gui::draw_info, gui::draw_behaviour) const;
		rects::wh<float> get_content_size() const;

		basic_handle<is_const, basic_pool<gui::rect>, gui::rect>  get_parent() const;

		void consume_raw_input_and_generate_gui_events(gui::raw_event_info&, gui::event_behaviour callback); /* event generator */
		void unhover(gui::raw_event_info& inf);
	};
}