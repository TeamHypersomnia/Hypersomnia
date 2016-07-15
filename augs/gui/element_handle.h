#pragma once
#include "rect_id.h"
#include "element_id.h"

namespace augs {
	namespace gui {
		struct element_meta;

		template<bool is_const, class element>
		class basic_element_handle : public basic_handle<is_const, basic_pool<element>, element> {
			typedef maybe_const_ref_t<is_const, rect_pool> rect_pool_ref;
			typedef maybe_const_ref_t<is_const, element_meta> meta_ref;
			
			rect_pool_ref rects;
			meta_ref meta;

		public:
			typedef element element_type;

			basic_element_handle(owner_reference owner, id_type elem, rect_pool_ref rects, meta_ref meta)
				: basic_handle(owner, elem), rects(rects), meta(meta) {
			}

			basic_rect_handle<is_const> get_rect() const {
				return rects[meta.tree_node];
			}

		};

		template<class element>
		using element_handle = basic_element_handle<false, element>;

		template<class element>
		using const_element_handle = basic_element_handle<true, element>;
	}
}