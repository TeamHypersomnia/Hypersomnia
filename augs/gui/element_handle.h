#pragma once
#include "rect_handle.h"
#include "element_id.h"

namespace augs {
	namespace gui {
		struct element_meta;
		
		template<class...>
		class element_world;

		template<bool is_const, class element>
		class element_handle_userdata {
		public:
		};

		template<class element, bool is_const, class... all_elements>
		class basic_element_handle_base : 
			public basic_handle<is_const, element_world<all_elements...>, element>,
			public default_rect_callbacks<is_const, basic_element_handle_base<element, is_const, all_elements...>>
		{
		public:
			typedef element element_type;

			using basic_handle::basic_handle;

			maybe_const_ref_t<is_const, rect_world> get_rect_world() const {
				return owner.rect_tree;
			}

			basic_rect_handle<is_const> get_rect() const {
				return owner.rects[get_meta<element_meta>().tree_node];
			}

		};

		template<class element, bool is_const, class... all_elements>
		class basic_element_handle : public basic_element_handle_base<element, is_const, all_elements...> {
		public:
			using basic_element_handle_base::basic_element_handle_base;
		};

		template<class element>
		using element_handle = basic_element_handle<element, false>;

		template<class element>
		using const_element_handle = basic_element_handle<element, true>;
	}
}