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

			pool_with_meta<rect, rect_meta> rects;
			tuple_of_t<make_pool_with_element_meta, all_elements...> element_pools;

		public:
			template<bool is_const, class... Args>
			class dispatcher {
				maybe_const_ref_t<is_const, element_world> elements;

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
			};
			
			rect_world rect_tree;

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

			template<class T, class... Args>
			element_handle<T> get_handle(pool_id<T> id, Args... args) {
				return{ get_pool(id), id, { args... }  };
			}

			template<class T, class... Args>
			const_element_handle<T> get_handle(pool_id<T> id, Args... args) const {
				return{ get_pool(id), id, { args... } };
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
			void dispatch_id(rect_id id, F f) {
				auto meta = rects.get_meta<rect_meta>(id);

				for_each_type<all_elements...>([this, meta, f](auto c) {
					auto typed_id = meta.get_id<decltype(c)>();

					if (get_pool(typed_id).alive(typed_id)) {
						f(typed_id);
					}
				});
			}

			void set_delta(fixed_delta delta) {
				rects.delta = delta;
			}

			fixed_delta get_delta() const {
				return rects.delta;
			}

			template<class T, class... Args> 
			element_id<T> create_element(const rect& new_node, Args... args) {
				auto& element_pool = get_pool(pool_id<T>());
				
				auto new_rect = rects.allocate(new_node);
				auto new_element = element_pool.allocate(rects[new_rect], *this, args...);
				
				auto& meta = rects.get_meta<rect_meta>(new_rect);
				auto& elem_meta = element_pool.get_meta<element_meta>(new_element);

				meta.get_id<T>() = new_element;
				elem_meta.tree_node = new_rect;

				return new_element;
			}

			void destroy_element(rect_id id) {
				dispatch_id(id, [this](auto id) {
					get_pool(id).free(id);
				});

				rects.free(id);
			}
		};
	}
}