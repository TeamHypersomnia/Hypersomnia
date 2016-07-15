#pragma once
#include "rect_id.h"
#include "element_id.h"

namespace augs {
	namespace gui {
		struct element_meta;

		template<bool is_const, class element>
		class basic_element_handle : public basic_handle<is_const, basic_pool<element>, element> {
		public:
			typedef element element_type;

			using basic_handle::basic_handle;
			//maybe_const_ref_t<is_const, rect_pool> rects;
			//maybe_const_ref_t<is_const, element_meta> meta;
		};

		template<class element>
		using element_handle = basic_element_handle<false, element>;

		template<class element>
		using const_element_handle = basic_element_handle<true, element>;
	}
}