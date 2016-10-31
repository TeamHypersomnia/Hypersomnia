#pragma once
#include "augs/templates.h"
#include "game/detail/gui/location_and_pointer.h"
#include "gui_element_location.h"

class viewing_step;
class logic_step;
class root_of_inventory_gui;

namespace component {
	struct gui_element;
}

class gui_tree_entry;

typedef std::unordered_map<gui_element_location, gui_tree_entry> gui_element_tree;

template <bool is_const>
class basic_dispatcher_context {

public:
	typedef std::conditional_t<is_const, viewing_step, logic_step>& step_ref;
	typedef maybe_const_ref_t<is_const, components::gui_element> gui_element_ref;
	typedef maybe_const_ref_t<is_const, game_gui_rect_world> game_gui_rect_world_ref;

	basic_dispatcher_context(step_ref step, basic_entity_handle<is_const> handle, gui_element_tree& tree, root_of_inventory_gui& root) :
		step(step),
		handle(handle),
		//composite_for_iteration(parent),
		elem(handle.get<components::gui_element>()),
		tree(tree),
		root(root)
	{}

	step_ref step;
	basic_entity_handle<is_const> handle;
	gui_element_ref elem;
	gui_element_tree& tree;
	root_of_inventory_gui& root;

	root_of_inventory_gui& get_root_of_inventory_gui() const {
		return root;
	}

	basic_entity_handle<is_const> get_gui_element_entity() const {
		return handle;
	}
	//parent_of_inventory_controls_rect& composite_for_iteration;

	step_ref get_step() const {
		return step;
	}

	gui_element_ref get_gui_element_component() const {
		return elem;
	}

	game_gui_rect_world_ref get_rect_world() const {
		return elem.rect_world;
	}

	gui_tree_entry& get_tree_entry(const gui_element_location& id) {
		if (tree.find(id) == tree.end()) {
			tree.emplace(id, gui_tree_entry(operator()(id, [](const auto& resolved_ref) {
				return static_cast<const augs::gui::rect_node_data&>(*resolved_ref);
			})));
		}

		return tree.at(id);
	}

	const gui_tree_entry& get_tree_entry(const gui_element_location& id) const {
		return tree.at(id);
	}

	bool alive(const gui_element_location& id) const {
		return id.is_set() && id.call([this](const auto& resolved) {
			return resolved.alive(*this);
		});
	}

	bool dead(const gui_element_location& id) const {
		return !alive(id);
	}

	operator basic_dispatcher_context<true>() const {
		return{ step, handle, elem, tree, root };
	}

	template <class L>
	decltype(auto) operator()(const gui_element_location& id, L generic_call) const {
		return id.call([&](const auto& resolved_location) {
			location_and_pointer<std::remove_pointer_t<decltype(resolved_location.dereference(*this))>> loc(resolved_location.dereference(*this), resolved_location);
			return generic_call(loc);
		});
	}

	template <class T, class L>
	decltype(auto) operator()(const location_and_pointer<T>& loc, L generic_call) const {
		return generic_call(loc);
	}
	//
	//template <bool C, class Casted>
	//struct pointer_caster {
	//	template <class Candidate>
	//	decltype(auto) operator()(Candidate& object) {
	//		if (std::is_same<Casted, Candidate>::value || std::is_base_of<Casted, Candidate>::value) {
	//			return reinterpret_cast<maybe_const_ptr_t<C, Casted>>(&object);
	//		}
	//
	//		return nullptr;
	//	}
	//
	//	template <class Candidate>
	//	const Casted* operator()(const Candidate& object) {
	//		if (std::is_same<Casted, Candidate>::value || std::is_base_of<Casted, Candidate>::value) {
	//			return reinterpret_cast<const Casted*>(&object);
	//		}
	//	
	//		return nullptr;
	//	}
	//};
	//
	//template <class T>
	//location_and_pointer<T> make_location_and_pointer(T* p, const typename T::location& l) const {
	//	return{ p, l };
	//}
	//
	//template <class T>
	//location_and_pointer<const T> make_location_and_pointer(const T* const p, const typename T::location& l) const {
	//	return{ p, l };
	//}

	template <class T>
	decltype(auto) dereference_location(const typename T::location& location) const {
		typedef typename T::location location_type;

		if (location.alive(*this)) {
			return location_and_pointer<T>(location.dereference(*this), location);
		}

		return location_and_pointer<T>();
	}

	template <class T>
	decltype(auto) _dynamic_cast(const gui_element_location& polymorphic_id) const {
		if (polymorphic_id.is<typename T::location>()) {
			return dereference_location<T>(polymorphic_id.get<typename T::location>());
		}

		return location_and_pointer<T>();
	}
};

typedef basic_dispatcher_context<false> dispatcher_context;
typedef basic_dispatcher_context<true> const_dispatcher_context;