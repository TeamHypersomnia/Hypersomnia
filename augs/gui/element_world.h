#pragma once
#include "templates.h"
#include "misc/object_pool.h"
#include "misc/object_pool_id.h"
#include "misc/object_pool_handlizer.h"
#include "rect_id.h"
#include "rect_world.h"
#include "element_handle.h"

namespace augs {
	namespace gui {
		template<class... all_elements>
		class element_world : public object_pool_handlizer<element_world<all_elements...>> {
			rect_world rects;

		public:
			typename transform_types<std::tuple, make_object_pool, all_elements...>::type element_pools;

			class rect_meta {
			public:
				typename transform_types<std::tuple, make_object_pool_id, all_elements...>::type element_ids;
			
			};

			template<class T>
			element_handle<T> get_handle(object_pool_id<T> id) {
				return{ *this, id };
			}

			template<class T>
			const_element_handle<T> get_handle(object_pool_id<T> id) const {
				return{ *this, id };
			}

			//template<>
			//rect_handle get_handle(rect_id id) {
			//	return rects[id];
			//}
			//
			//template<>
			//const_rect_handle get_handle(rect_id id) const {
			//	return rects[id];
			//}

			//template<class T>
			//element_handle<T> downcast(rect_id) {
			//
			//}
			//
			//template<class T>
			//const_element_handle<T> downcast(rect_id) const {
			//
			//}

			std::vector<rect_meta> elements;

			template<class element>
			void create_element() {

			}

			void destroy_element(rect_id) {

			}
		};
	}
}