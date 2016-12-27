#pragma once
#include "augs/gui/dereferenced_location.h"
#include "augs/gui/rect_tree.h"
#include "augs/gui/rect_world.h"
#include "augs/templates/maybe_const.h"

namespace augs {
	namespace gui {
		template <class gui_element_polymorphic_id, bool is_const_value, class derived>
		class basic_context {
		public:
			static constexpr bool is_const = is_const_value;
			
			typedef maybe_const_ref_t<is_const_value, rect_world<gui_element_polymorphic_id>> rect_world_ref;
			typedef maybe_const_ref_t<false, rect_tree<gui_element_polymorphic_id>> tree_ref;
			typedef maybe_const_ref_t<false, rect_tree_entry<gui_element_polymorphic_id>> rect_tree_entry_ref;
			
			rect_world_ref world;
			tree_ref tree;
			
			basic_context(rect_world_ref world, tree_ref tree) : world(world), tree(tree) {}

			rect_world_ref get_rect_world() const {
				return world;
			}

			rect_tree_entry_ref get_tree_entry(const gui_element_polymorphic_id& id) const {
				return tree.at(id);
			}
			
			template<class id_type, bool _is_const = is_const_ref<rect_tree_entry_ref>::value, class = std::enable_if_t<!_is_const>>
			rect_tree_entry_ref make_tree_entry(const id_type& id) const {
				//ensure(tree.find(id) == tree.end()) 
				
				return (*tree.emplace(id, operator()(id, [](const auto resolved_ref) {
					return resolved_ref->rc;
				})).first).second;
			}

			bool alive(const gui_element_polymorphic_id& id) const {
				return id.is_set() && id.call([this](const auto resolved) {
					return resolved.alive(*static_cast<const derived*>(this));
				});
			}

			bool dead(const gui_element_polymorphic_id& id) const {
				return !alive(id);
			}

			bool alive(gui_element_polymorphic_id& id) const {
				const auto& const_id = id;

				if (!alive(const_id)) {
					id.unset();

					return false;
				}

				return true;
			}

			bool dead(gui_element_polymorphic_id& id) const {
				return !alive(id);
			}

			template <class T>
			auto dereference_location(const T& location) const {
				const auto& self = *static_cast<const derived*>(this);
				
				typedef decltype(location.dereference(self)) dereferenced_ptr;

				if (location.alive(self)) {
					return make_dereferenced_location( location.dereference(self), location );
				}

				return make_dereferenced_location(static_cast<dereferenced_ptr>(nullptr), location );
			}

			template <class L>
			decltype(auto) operator()(const gui_element_polymorphic_id& id, L generic_call) const {
				return id.call([&](const auto specific_loc) {
					return generic_call(dereference_location(specific_loc));
				});
			}

			template <bool is_const, class T, class L>
			decltype(auto) operator()(const basic_dereferenced_location<is_const, T>& loc, L generic_call) const {
				return generic_call(loc);
			}

			template <class T>
			auto _dynamic_cast(const gui_element_polymorphic_id& polymorphic_id) const {
				if (polymorphic_id.is<T>()) {
					return dereference_location(polymorphic_id.get<T>());
				}

				return dereference_location(T());
			}
		};
	}
}