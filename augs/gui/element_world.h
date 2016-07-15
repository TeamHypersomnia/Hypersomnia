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
		struct make_pool_with_element_meta { typedef pool_with_meta<T, element_meta> type; };

		template<class... all_elements>
		class element_world : public pool_handlizer<element_world<all_elements...>> {
			class rect_meta {
			public:
				tuple_of_t<make_pool_id, all_elements...> element_ids;

				template<class T>
				T& get_id() {
					return std::get<element_id<T>>(element_ids);
				}
			};

			template<bool C, class D, class... E>
			friend class basic_element_handle_base<C, D, E...>;

			rect_world rect_tree;
			pool_with_meta<rect, rect_meta> rects;
			tuple_of_t<make_pool_with_element_meta, all_elements...> element_pools;

		public:
			element_world() {
				auto new_rect = rects.allocate();
				auto& r = new_rect.get();

				r.clip = false;
				r.focusable = false;
				r.scrollable = false;

				rect_tree.root = new_rect;
			}

			template <class T>
			auto& get_pool(pool_id<T>) {
				return std::get<pool_with_meta<T, element_meta>>(element_pools);
			}

			template <class T>
			const auto& get_pool(pool_id<T>) const {
				return std::get<pool_with_meta<T, element_meta>>(element_pools);
			}

			template<class T>
			element_handle<T> get_handle(pool_id<T> id) {
				return{ get_pool(id), id };
			}

			template<class T>
			const_element_handle<T> get_handle(pool_id<T> id) const {
				return{ get_pool(id), id };
			}

			template<>
			rect_handle get_handle(rect_id id) {
				return rects[id];
			}
			
			template<>
			const_rect_handle get_handle(rect_id id) const {
				return rects[id];
			}

			template <class F>
			void dispatch(rect_id id, F f) {
				auto meta = rects.get_meta<rect_meta>(id);

				for_each_type<all_elements...>([this, meta, f](auto c) {
					auto element_handle = get_handle(meta.get_id<decltype(c)>());

					if (element_handle.alive()) {
						f(element_handle);
					}
				});
			}

			void set_delta(fixed_delta delta) {
				rects.delta = delta;
			}

			fixed_delta get_delta() const {
				return rects.delta;
			}

			template<class T, class = std::enable_if_t<!std::is_same<T, rect>::value>>
			element_handle<T> create_element() {
				auto& element_pool = get_pool(pool_id<T>());
				
				auto new_rect = rects.allocate();
				auto new_element = element_pool.allocate();
				
				auto& meta = rects.get_meta<rect_meta>(new_rect);
				auto& elem_meta = element_pool.get_meta<element_meta>(new_element);

				meta.get_id<T>() = new_element;
				elem_meta.tree_node = new_rect;

				return new_element;
			}

			void destroy_element(rect_id id) {
				dispatch(id, [this](auto c) {
					c.get_pool().free(c.get_id());
				});

				rects.free(id);
			}
		};
	}
}