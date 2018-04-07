#include "augs/misc/simple_pair.h"
#include "application/setups/editor/gui/editor_all_entities_gui.h"
#include "application/setups/editor/editor_command_input.h"

void editor_all_entities_gui::open() {
	show = true;
	acquire_once = true;
	ImGui::SetWindowFocus(title.c_str());
}

void editor_all_entities_gui::interrupt_tweakers() {
	properties_gui.last_active.reset();
	properties_gui.old_description.clear();
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

#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/byte_readwrite.h"

using resolved_array_type = per_entity_type_array<
	std::unordered_map<raw_entity_flavour_id, std::vector<entity_id_base>>
>;

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
	const resolved_array_type& per_native_type;

	template <class E>
	const auto& get_map() const {
		return per_native_type[entity_type_id::of<E>().get_index()];
	}

public:
	in_selection_provider(
		const cosmos& cosm,
		const resolved_array_type& per_native_type
	) : 
		cosm(cosm),
		per_native_type(per_native_type)
	{}

	bool skip_empty_nodes() const {
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
		thread_local std::vector<raw_entity_flavour_id> ids;
		ids.clear();

		for (const auto& p : get_map<E>()) {
			ids.push_back(p.first);
		}

		return ids;
	}

	template <class E>
	auto get_entities_by_flavour_id(const raw_entity_flavour_id raw) const {
		thread_local std::vector<entity_id_base> ids;
		ids.clear();
		ids	= get_map<E>().at(raw);

		sort_range_by(
			ids,
			[&](const auto id) { 
				return cosm[typed_entity_id<E>(id)].get_guid();
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

	bool skip_empty_nodes() const {
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
		thread_local std::vector<raw_entity_flavour_id> all_flavour_ids;
		all_flavour_ids.clear();

		const auto& all_flavours = common().get_flavours<E>();

		all_flavours.for_each([&](
			const auto flavour_id,
			const auto& flavour
		) {
			all_flavour_ids.push_back(flavour_id.raw);
		});

		return all_flavour_ids;
	}

	template <class E>
	const auto& get_entities_by_flavour_id(const raw_entity_flavour_id id) const {
		return cosm.get_solvable_inferred().name.get_entities_by_flavour_id(typed_entity_flavour_id<E>(id));
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
	const editor_settings& settings,
	const std::unordered_set<entity_id>* only_match_entities,
	editor_command_input in
) {
	if (!show) {
		return {};
	}

	using namespace augs::imgui;

	ImGui::SetNextWindowSize(ImVec2(350,560), ImGuiCond_FirstUseEver);

	auto entities = scoped_window(title.c_str(), &show);

	ImGui::Columns(2);
	ImGui::Separator();

	if (acquire_once) {
		ImGui::SetKeyboardFocusHere();
		acquire_once = false;
	}

	properties_gui.hovered_guid.unset();

	const bool show_filter_buttons = only_match_entities != nullptr;

	const auto prop_in = property_editor_input { settings.property_editor, properties_gui };

	const auto fae_in = fae_tree_input { 
		prop_in, show_filter_buttons
	};

	const auto& cosm = in.get_cosmos();

	if (only_match_entities) {
		const auto& matches = *only_match_entities;
		const auto num_matches = matches.size();

		if (num_matches == 0) {
			return {};
		}
		else if (num_matches == 1) {
			const auto id = *matches.begin();

			if (const auto handle = cosm[id]) {
				handle.dispatch([&](const auto typed_handle) {
					do_edit_flavours_gui(prop_in, in, typed_handle.get_flavour(), { typed_handle.get_flavour_id().raw });
					do_edit_entities_gui(prop_in, in, typed_handle, { id.basic() });
				});
			}

			return {};
		}

		thread_local resolved_array_type per_native_type;

		for (auto& p : per_native_type) {
			p.clear();
		}

		for (const auto& e : matches) {
			if (const auto handle = cosm[e]) {
				per_native_type[e.type_id.get_index()][handle.get_flavour_id().raw].push_back(e);
			}
		}

		return fae_tree(
			fae_in,
			in,
			in_selection_provider { cosm, per_native_type }
		);
	}

	return fae_tree(
		fae_in,
		in,
		all_provider { cosm }
	);
}

#else
fae_tree_filter editor_all_entities_gui::perform(
	const editor_settings& settings,
	const std::unordered_set<entity_id>* only_match_entities,
	editor_command_input in
) {
	return {};
}
#endif
