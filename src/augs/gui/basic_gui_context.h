#pragma once
#include "augs/gui/dereferenced_location.h"
#include "augs/gui/rect_tree.h"
#include "augs/gui/rect_world.h"
#include "augs/gui/gui_traversal_structs.h"
#include "augs/templates/maybe_const.h"
#include "augs/templates/container_templates.h"

namespace augs {
	namespace gui {
		template <class T = float>
		struct drawing_rects {
			basic_ltrb<T> absolute;
			basic_ltrb<T> clipper;
		};

		template <class gui_element_variant_id, bool is_const, class derived>
		class basic_context {
		public:
			using tree_type = rect_tree<gui_element_variant_id>;
			using rect_world_type = rect_world<gui_element_variant_id>;

			using rect_world_ref = maybe_const_ref_t<is_const, rect_world_type>;
			using tree_ref = maybe_const_ref_t<is_const, tree_type>;
			using rect_tree_entry_type = rect_tree_entry<gui_element_variant_id>;

			rect_world_ref world;
			tree_ref tree;
			const vec2i screen_size;
			const event::state input_state;
			
			basic_context(
				rect_world_ref world, 
				tree_ref tree,
				const vec2i screen_size,
				const event::state input_state
			) : 
				world(world), 
				tree(tree),
				screen_size(screen_size),
				input_state(input_state)
			{}

			template <class other_derived>
			operator basic_context<gui_element_variant_id, true, other_derived>() const {
				return { world, tree, screen_size, input_state };
			};

			auto get_input_state() const {
				return input_state;
			};

			rect_world_ref get_rect_world() const {
				return world;
			}

			rect_tree_entry_type get_tree_entry(const gui_element_variant_id& id) const {
				const auto& self = *static_cast<const derived*>(this);

				if (const auto found = mapped_or_nullptr(tree, id)) {
					return *found;
				}

				return { ltrb {}, self.get_root_id() };
			}

			auto get_screen_size() const {
				return screen_size;
			}

			bool alive(const gui_element_variant_id& id) const {
				return 
					!(id == gui_element_variant_id()) 
					&& std::visit([this](const auto resolved) {
						return resolved.alive(*static_cast<const derived*>(this));
					}, id)
				;
			}

			bool dead(const gui_element_variant_id& id) const {
				return !alive(id);
			}

			template <class T>
			auto dereference_location(const T& location) const {
				const auto& self = *static_cast<const derived*>(this);
				
				using dereferenced_ptr = decltype(location.dereference(self));

				if (location.alive(self)) {
					return make_dereferenced_location(location.dereference(self), location);
				}

				return make_dereferenced_location(static_cast<dereferenced_ptr>(nullptr), location);
			}

			template <class L>
			decltype(auto) operator()(const gui_element_variant_id& id, L generic_call) const {
				return std::visit([&](const auto specific_loc) -> decltype(auto) {
					return generic_call(dereference_location(specific_loc));
				}, id);
			}

			template <bool _is_const, class T, class L>
			decltype(auto) operator()(const basic_dereferenced_location<_is_const, T>& loc, L generic_call) const {
				return generic_call(loc);
			}

			template <class T>
			auto get_if(const gui_element_variant_id& variant_id) const {
				auto* const maybe_t = std::get_if<T>(&variant_id);
				
				using dereferenced_location_type = decltype(dereference_location(*maybe_t));

				if (maybe_t) {
					return dereference_location(*maybe_t);
				}

				return dereferenced_location_type();
			}

			template <class T = float>
			auto get_drawing_rects(const gui_element_variant_id& id) const {
				const auto entry = get_tree_entry(id);

				return drawing_rects<T>{
					entry.get_absolute_rect(),
					get_tree_entry(entry.get_parent()).get_absolute_clipping_rect()
				};
			}
		};

#if 0
		template <class gui_element_variant_id, class derived>
		class basic_viewing_context : public basic_context<gui_element_variant_id, true, derived> {
		public:
			basic_viewing_context(
				const base b,
				const augs::drawer_with_default output
			) : 
				base(b),
				output(output)
			{}

			const augs::drawer_with_default output;

			auto get_output() const {
				return output;
			}
		};
#endif
	}
}