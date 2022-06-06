#define INCLUDE_TYPES_IN 1

#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "application/setups/debugger/gui/debugger_fae_gui.h"
#include "application/setups/debugger/debugger_command_input.h"

void debugger_fae_gui_base::interrupt_tweakers() {
	property_debugger_data.reset();
}

#if BUILD_PROPERTY_DEBUGGER

#include "augs/templates/for_each_std_get.h"
#include "augs/misc/simple_pair.h"

#include "augs/readwrite/memory_stream.h"

#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_enum_radio.h"

#include "game/cosmos/for_each_entity.h"

#include "application/intercosm.h"
#include "application/setups/debugger/debugger_folder.h"

#include "application/setups/debugger/property_debugger/fae/fae_tree.h"
#include "application/setups/debugger/property_debugger/commanding_property_debugger_input.h"
#include "application/setups/debugger/debugger_history.hpp"
#include "application/setups/debugger/detail/sort_flavours_by_name.h"

#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/byte_readwrite.h"

using ticked_flavours_type = debugger_fae_gui_base::ticked_flavours_type;
using ticked_entities_type = debugger_fae_gui_base::ticked_entities_type;

void debugger_fae_gui_base::do_view_mode_switch() {
	using namespace augs::imgui;

	auto child = scoped_child("fae-view-switch");
	ImGui::Separator();
	enum_radio(view_mode, true);
}

class in_selection_provider {
	const cosmos& cosm;
	const flavour_to_entities_type& flavour_to_entities;

	const bool skip_empty;

	template <class E>
	const auto& get_map() const {
		return flavour_to_entities.get_for<E>();
	}

public:
	in_selection_provider(
		const cosmos& cosm,
		const flavour_to_entities_type& flavour_to_entities,
		const bool skip_empty
	) : 
		cosm(cosm),
		flavour_to_entities(flavour_to_entities),
		skip_empty(skip_empty)
	{}

	bool skip_nodes_with_no_entities() const {
		return true;
	}

	template <class E>
	auto num_flavours_of_type() const {
		return get_map<E>().size();
	}

	template <class E>
	auto num_entities_of_type() const {
		std::size_t total = 0;

		for (const auto& p : get_map<E>()) {
			total += p.second.size();
		}

		return total;
	}

	template <class E>
	bool should_skip_type() const {
		return skip_empty && (num_entities_of_type<E>() == 0 && num_flavours_of_type<E>() == 0);
	}

	template <class E>
	auto get_entities_by_flavour_id(const typed_entity_flavour_id<E> id) const {
		thread_local std::vector<typed_entity_id<E>> ids;
		ids	= get_map<E>().at(id);
		sort_range(ids);
		return ids;
	}

	template <class E, class F>
	void for_each_flavour(F callback) const {
		thread_local std::vector<typed_entity_flavour_id<E>> ids;
		ids.clear();

		const auto& all_flavours = get_map<E>();

		for (const auto& f : all_flavours) {
			const auto id = typed_entity_flavour_id<E>(f.first);
			ids.push_back(id);
		}

		sort_flavours_by_name(cosm, ids);

		for (const auto& id : ids) {
			const auto& flavour = cosm.get_flavour(id);
			callback(id, flavour);
		}
	}

	template <class T>
	auto get_affected_for(typed_entity_flavour_id<T> id) {
		return std::vector<decltype(id)> { id };
	}

	template <class T>
	auto get_affected_for(typed_entity_id<T> id) {
		return std::vector<decltype(id)> { id };
	}
};

fae_tree_input debugger_fae_gui_base::make_fae_input(
	const debugger_fae_gui_input in,
	const bool is_selection
) {
	const auto prop_in = property_debugger_input { 
		in.settings,
		property_debugger_data 
	};

	const auto command_in = in.command_in;
	const auto cpe_in = commanding_property_debugger_input {
		prop_in, command_in
	};

	return fae_tree_input { 
		cpe_in, is_selection, !is_selection, !is_selection, !is_selection
	};
}

template <class F>
static void populate(
	flavour_to_entities_type& fte,
	ImGuiTextFilter& filter,
	F for_each_handle
) {
	fte.clear();

	for_each_handle(
		[&](const auto typed_handle) {
			const auto name = typed_handle.get_name();

			if (!filter.PassFilter(name.c_str())) {
				return;
			}

			using E = entity_type_of<decltype(typed_handle)>;
			fte.get_for<E>()[typed_handle.get_flavour_id()].push_back(typed_handle.get_id());
		}
	);
}

template <class F, class C>
void for_each_typed_handle_in(const cosmos& cosm, const C& container, F&& callback) {
	for (const auto& e : container) {
		if (const auto handle = cosm[e]) {
			handle.dispatch(std::forward<F>(callback));
		}
	}
}

#endif
fae_tree_output debugger_selected_fae_gui::perform(
	const debugger_fae_gui_input in,
	const fae_selections_type& matches
) {
#if BUILD_PROPERTY_DEBUGGER
	using namespace augs::imgui;

	auto entities = make_scoped_window();

	if (!entities) {
		return {};
	}

	entities_tree_data.hovered_id.unset();

	const auto& cosm = in.command_in.get_cosmos();

	const auto fae_in = make_fae_input(in, true);

	if (matches.empty()) {
		return {};
	}
	else if (matches.size() == 1) {
		ImGui::Columns(2);

		const auto id = *matches.begin();

		if (const auto handle = cosm[id]) {
			handle.dispatch([&](const auto typed_handle) {
				do_edit_flavours_gui(fae_in, typed_handle.get_flavour(), typed_handle.get_flavour_id());
				ImGui::Separator();
				do_edit_entities_gui(fae_in, typed_handle, typed_handle.get_id());
			});
		}

		return {};
	}

	switch (view_mode) {
		case fae_view_type::ENTITIES:
			filter.Draw("Filter flavours & entities");
			break;
		case fae_view_type::FLAVOURS:
			filter.Draw("Filter flavours");
			break;

		default: 
			break;
	}

	auto for_each_match = 
		[&](auto&& callback) {
			for_each_typed_handle_in(cosm, matches, std::forward<decltype(callback)>(callback));
		}
	;

	erase_if(ticked_entities, [&](const auto& id) {
		return !found_in(matches, entity_id(id));
	});

	{
		thread_local ticked_flavours_type all_matched_flavours;
		all_matched_flavours.clear();

		for_each_match([&](const auto typed_handle) {
			using E = decltype(typed_handle);
			all_matched_flavours.get_for<entity_type_of<E>>().emplace(typed_handle.get_flavour_id());
		});

		erase_if(ticked_flavours, [&](const auto& id) {
			using E = entity_type_of<decltype(id)>;
			return !found_in(all_matched_flavours.get_for<E>(), id);
		});
	}

	populate(
		cached_flavour_to_entities,
		filter,
		for_each_match
	);

	auto after_return = augs::scope_guard([&]() { 
		do_view_mode_switch(); 
	});

	const auto provider = in_selection_provider { cosm, cached_flavour_to_entities, true };

	switch (view_mode) {
		case fae_view_type::ENTITIES:
			return tree_of_entities(fae_in, entities_tree_data, provider, ticked_entities);
		case fae_view_type::FLAVOURS:
			return tree_of_flavours(fae_in, provider, ticked_flavours);

		default: 
			return {};
	}
#else
	(void)in;
	(void)matches;
	return {};
#endif
}

fae_tree_output debugger_fae_gui::perform(
	const debugger_fae_gui_input in, 
	fae_selections_type& all_selections
) {
#if BUILD_PROPERTY_DEBUGGER
	using namespace augs::imgui;

	auto entities = make_scoped_window();

	if (!entities) {
		return {};
	}

	entities_tree_data.hovered_id.unset();

	const auto fae_in = make_fae_input(in, false);

	const auto& cosm = in.command_in.get_cosmos();

	erase_if(ticked_flavours, [&](const auto& id) {
		return cosm.find_flavour(id) == nullptr;
	});

	switch (view_mode) {
		case fae_view_type::ENTITIES:
			filter.Draw("Filter flavours & entities");
			break;
		case fae_view_type::FLAVOURS:
			filter.Draw("Filter flavours");
			break;

		default: 
			break;
	}

	populate(
		cached_flavour_to_entities,
		filter,
		[&cosm](auto&& callback) {
			cosm.for_each_entity(std::forward<decltype(callback)>(callback));
		}
	);

	for_each_entity_type([&](auto e){
		cosm.for_each_id_and_flavour<decltype(e)>([&](const auto id, const auto& f) {
			if (filter.PassFilter(f.get_name().c_str())) {
				cached_flavour_to_entities.get_for<entity_type_of<decltype(id)>>().try_emplace(id);
			}
		});
	});

	cached_ticked_entities.clear();

	for_each_typed_handle_in(cosm, all_selections, [&](const auto typed_handle) {
		using E = entity_type_of<decltype(typed_handle)>;
		cached_ticked_entities.get_for<E>().emplace(typed_handle.get_id());
	});

	const auto provider = in_selection_provider { cosm, cached_flavour_to_entities, !filter.Filters.empty() };

	auto after_return = augs::scope_guard([&]() { 
		all_selections.clear();

		cached_ticked_entities.for_each([&all_selections](const auto id) {
			all_selections.emplace(entity_id(id));
		});

		do_view_mode_switch(); 
	});

	switch (view_mode) {
		case fae_view_type::ENTITIES:
			return tree_of_entities(fae_in, entities_tree_data, provider, cached_ticked_entities);
		break;

		case fae_view_type::FLAVOURS:
			return tree_of_flavours(fae_in, provider, ticked_flavours);
		break;

		default: 
			return {};
		break;
	}
#else
	(void)in;
	(void)all_selections;
	return {};
#endif
}

void debugger_fae_gui_base::clear_dead_entities(const cosmos& cosm) {
	cosm.clear_dead(entities_tree_data.hovered_id);
}

void debugger_fae_gui::clear_dead_entities(const cosmos& cosm) {
	debugger_fae_gui_base::clear_dead_entities(cosm);
	cosm.erase_dead(cached_ticked_entities);
}

void debugger_selected_fae_gui::clear_dead_entities(const cosmos& cosm) {
	debugger_fae_gui_base::clear_dead_entities(cosm);
	cosm.erase_dead(ticked_entities);
}
