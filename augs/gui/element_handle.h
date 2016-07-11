#pragma once
#include "rect_id.h"
#include "element_id.h"

namespace augs {
	namespace gui {
		template<bool is_const, class element>
		class basic_element_handle {
		public:
			operator rect_id() const;
			operator element_id<element>() const;
		};

		template<class element>
		using element_handle = basic_element_handle<false, element>;

		template<class element>
		using const_element_handle = basic_element_handle<true, element>;
	}
}