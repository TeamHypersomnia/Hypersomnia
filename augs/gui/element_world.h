#pragma once
#include "templates.h"
#include "misc/pool.h"
#include "misc/pool_id.h"
#include "misc/pool_handlizer.h"
#include "rect_id.h"
#include "rect_world.h"
#include "element_handle.h"
#include "misc/delta.h"

namespace augs {
	namespace gui {
		struct element_meta {
			rect_id tree_node;
		};

		template<class T>
		struct make_pool_with_element_meta { typedef pool<T, element_meta> type; };

		template<class... all_elements>
		class element_world : public pool_handlizer<element_world<all_elements...>> {
			class rect_meta {
			public:
				tuple_of_t<make_pool_id, all_elements...> element_ids;
			};

			rect_world rect_tree;
			pool_with_meta<rect, rect_meta> rects;

		public:
			tuple_of_t<make_pool_with_element_meta, all_elements...> element_pools;

			element_world() {
				auto new_rect = rects.allocate();
				auto& r = new_rect.get();

				r.clip = false;
				r.focusable = false;
				r.scrollable = false;

				rect_tree.root = new_rect;
			}
	
			template<class T>
			element_handle<T> get_handle(pool_id<T> id) {
				return{ *this, id };
			}

			template<class T>
			const_element_handle<T> get_handle(pool_id<T> id) const {
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

			void set_delta(fixed_delta delta) {
				rects.delta = delta;
			}

			fixed_delta get_delta() const {
				return rects.delta;
			}


			template<class element>
			void create_element() {

			}

			void destroy_element(rect_id) {

			}
		};
	}
}