#pragma once
#include "augs/templates/maybe_const.h"
#include "game/detail/gui/location_and_pointer.h"
#include "gui_element_location.h"

class root_of_inventory_gui;

namespace component {
	struct gui_element;
}

class gui_tree_entry;

typedef std::unordered_map<gui_element_location, gui_tree_entry> gui_element_tree;

template <class step_type>
class basic_gui_context {

public:
	typedef std::decay_t<decltype(std::declval<step_type>().get_cosmos().get_handle(entity_id()))> entity_handle_type;
	static constexpr bool is_const = entity_handle_type::is_const_value;

	typedef maybe_const_ref_t<is_const, components::gui_element> gui_element_ref;
	typedef maybe_const_ref_t<is_const, game_gui_rect_world> game_gui_rect_world_ref;
	typedef maybe_const_ref_t<is_const, root_of_inventory_gui> root_of_inventory_gui_ref;

	basic_gui_context(step_type step, entity_handle_type handle, gui_element_tree& tree, root_of_inventory_gui_ref root) :
		step(step),
		handle(handle),
		elem(handle.get<components::gui_element>()),
		tree(tree),
		root(root)
	{}

	step_type step;
	entity_handle_type handle;
	gui_element_ref elem;
	gui_element_tree& tree;
	root_of_inventory_gui_ref root;

	template<class other_context>
	operator other_context() const {
		return other_context(step, handle, tree, root);
	}

	root_of_inventory_gui_ref get_root_of_inventory_gui() const {
		return root;
	}

	basic_entity_handle<is_const> get_gui_element_entity() const {
		return handle;
	}

	step_type get_step() const {
		return step;
	}

	gui_element_ref get_gui_element_component() const {
		return elem;
	}

	game_gui_rect_world_ref get_rect_world() const {
		return elem.rect_world;
	}

	gui_tree_entry& get_tree_entry(const gui_element_location& id) const {
		if (tree.find(id) == tree.end()) {
			tree.emplace(id, gui_tree_entry(operator()(id, [](const auto& resolved_ref) {
				return static_cast<const augs::gui::rect_node_data&>(*resolved_ref);
			})));
		}

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

	template <class L>
	decltype(auto) operator()(const gui_element_location& id, L generic_call) const {
		return id.call([&](const auto& specific_loc) {
			return generic_call(
				location_and_pointer<std::remove_pointer_t<decltype(specific_loc.dereference(*this))>>(specific_loc.dereference(*this), specific_loc)
			);
		});
	}

	template <class T, class L>
	decltype(auto) operator()(const location_and_pointer<T>& loc, L generic_call) const {
		return generic_call(loc);
	}

	template <class T>
	auto dereference_location(const T& location) const 
	-> location_and_pointer<std::remove_pointer_t<decltype(std::declval<T>().dereference(*this))>>
	{
		if (location.alive(*this)) {
			return{ location.dereference(*this), location };
		}

		return {};
	}

	template <class T>
	auto _dynamic_cast(const gui_element_location& polymorphic_id) const 
		-> location_and_pointer<std::remove_pointer_t<decltype(std::declval<typename T::location>().dereference(*this))>>
	{
		if (polymorphic_id.is<typename T::location>()) {
			return dereference_location(polymorphic_id.get<typename T::location>());
		}

		return{};
	}
};

class viewing_step;
class logic_step;
template<bool is_const>
class basic_cosmic_step;

typedef basic_cosmic_step<true> const_cosmic_step;

typedef basic_gui_context<logic_step> logic_gui_context;
typedef basic_gui_context<const_cosmic_step> const_gui_context;
typedef basic_gui_context<viewing_step> viewing_gui_context;
