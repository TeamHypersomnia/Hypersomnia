#pragma once
#include "rect_id.h"
#include "element_id.h"

namespace augs {
	namespace gui {
		template<bool is_const, class element>
		class basic_element_handle<is_const, rect_world, rect> {
		public:
			template<class T, class = std::enable_if<std::is_base_of<T, element>::value>::type>
			operator element_id<T>() const {
				element_id<T> id;

			}
		};

		template<class element>
		using element_handle = basic_element_handle<false, element>;

		template<class element>
		using const_element_handle = basic_element_handle<true, element>;
	}
}