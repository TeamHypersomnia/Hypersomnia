#include "augs/misc/simple_pair.h"
#include "application/setups/editor/gui/editor_all_entities_gui.h"
#include "application/setups/editor/editor_command_input.h"

void editor_all_entities_gui::interrupt_tweakers() {
	property_editor_data.last_active.reset();
	property_editor_data.old_description.clear();
}

#if BUILD_PROPERTY_EDITOR

#include "augs/templates/for_each_std_get.h"

#include "augs/readwrite/memory_stream.h"

#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "application/intercosm.h"
#include "application/setups/editor/editor_folder.h"

#include "application/setups/editor/property_editor/fae_tree.h"
#include "application/setups/editor/property_editor/commanding_property_editor_input.h"

#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/byte_readwrite.h"

template <class E>
using make_flavour_to_entities_map = std::unordered_map<typed_entity_flavour_id<E>, std::vector<typed_entity_id<E>>>;
using resolved_container_type = per_entity_type_container<make_flavour_to_entities_map>;

template <class C>
void sort_flavours_by_name(const cosmos& cosm, C& ids) {
	sort_range_by(
		ids,
		[&](const auto id) -> const std::string* { 
			return std::addressof(cosm.get_flavour(id).template get<invariants::name>().name);
		},
		[](const auto& a, const auto& b) {
			return *a.compared < *b.compared;
		}
	);
}

class in_selection_provider {
	const cosmos& cosm;
	const resolved_container_type& per_native_type;

	template <class E>
	const auto& get_map() const {
		return per_native_type.get_for<E>();
	}

public:
	in_selection_provider(
		const cosmos& cosm,
		const resolved_container_type& per_native_type
	) : 
		cosm(cosm),
		per_native_type(per_native_type)
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
	const auto& get_all_flavour_ids() const {
		thread_local std::vector<typed_entity_flavour_id<E>> ids;
		ids.clear();

		for (const auto& p : get_map<E>()) {
			ids.push_back(p.first);
		}

		return ids;
	}

	template <class E>
	auto get_entities_by_flavour_id(const typed_entity_flavour_id<E> id) const {
		thread_local std::vector<typed_entity_id<E>> ids;
		ids	= get_map<E>().at(id);

		sort_range_by(
			ids,
			[&](const auto id) { 
				return cosm[id].get_guid();
			}
		);

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
};

class all_provider {
	const cosmos& cosm;

	const auto& common() const {
		return cosm.get_common_significant();
	}

public:
	all_provider(const cosmos& cosm) : cosm(cosm) {}

	bool skip_nodes_with_no_entities() const {
		return false;
	}

	template <class E>
	auto num_flavours_of_type() const {
		return common().get_flavours<E>().count();
	}

	template <class E>
	auto num_entities_of_type() const {
		return cosm.get_solvable().get_count_of<E>();
	}

	template <class E>
	const auto& get_all_flavour_ids() const {
		thread_local std::vector<typed_entity_flavour_id<E>> all_flavour_ids;
		all_flavour_ids.clear();

		const auto& all_flavours = common().get_flavours<E>();

		all_flavours.for_each([&](const auto flavour_id, const auto& flavour) {
			all_flavour_ids.push_back(flavour_id);
		});

		return all_flavour_ids;
	}

	template <class E>
	const auto& get_entities_by_flavour_id(const typed_entity_flavour_id<E> id) const {
		return cosm.get_solvable_inferred().name.get_entities_by_flavour_id(id);
	}

	template <class E, class F>
	void for_each_flavour(F callback) const {
		thread_local std::vector<typed_entity_flavour_id<E>> ids;
		ids.clear();
		ids.reserve(num_flavours_of_type<E>());

		const auto& all_flavours = common().get_flavours<E>();

		all_flavours.for_each([](const auto& id, const auto&){
			ids.push_back(id);	
		});

		sort_flavours_by_name(cosm, ids);

		for (const auto& id : ids) {
			callback(id, cosm.get_flavour(id));
		}
	}
};

fae_tree_filter editor_all_entities_gui::perform(
	const editor_all_entities_gui_input in
) {
	using namespace augs::imgui;

	auto entities = make_scoped_window();

	if (!entities) {
		return {};
	}

	ImGui::Columns(2);
	ImGui::Separator();

	fae_tree_data.hovered_guid.unset();

	const bool show_filter_buttons = in.only_match_entities != nullptr;

	const auto prop_in = property_editor_input { 
		in.settings,
		property_editor_data 
	};

	const auto command_in = in.command_in;

	const auto cpe_in = commanding_property_editor_input {
		prop_in, command_in
	};

	const auto fae_in = fae_tree_input { 
		fae_tree_data, cpe_in, show_filter_buttons, in.image_caches
	};

	const auto& cosm = command_in.get_cosmos();

	if (in.only_match_entities != nullptr) {
		const auto& matches = *in.only_match_entities;
		const auto num_matches = matches.size();

		if (num_matches == 0) {
			return {};
		}
		else if (num_matches == 1) {
			const auto id = *matches.begin();

			if (const auto handle = cosm[id]) {
				handle.dispatch([&](const auto typed_handle) {
					do_edit_flavours_gui(fae_in, typed_handle.get_flavour(), { typed_handle.get_flavour_id() });
					do_edit_entities_gui(fae_in, typed_handle, { typed_handle.get_id() });
				});
			}

			return {};
		}

		thread_local resolved_container_type per_native_type;
		per_native_type.clear();

		for (const auto& e : matches) {
			if (const auto handle = cosm[e]) {
				handle.dispatch([](const auto typed_handle) {
					using E = entity_type_of<decltype(typed_handle)>;
					per_native_type.get_for<E>()[typed_handle.get_flavour_id()].push_back(typed_handle.get_id());
				});
			}
		}

		return fae_tree(
			fae_in,
			in_selection_provider { cosm, per_native_type }
		);
	}

	return fae_tree(
		fae_in,
		all_provider { cosm }
	);
}

#else
fae_tree_filter editor_all_entities_gui::perform(
	const editor_all_entities_gui_input in
) {
	return {};
}
#endif
