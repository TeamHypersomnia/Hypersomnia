#pragma once
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/templates/identity_templates.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "view/viewables/all_viewables_defs.h"

#include "augs/misc/imgui/path_tree_structs.h"

#include "application/setups/debugger/detail/get_id_or_import.h"
#include "application/setups/debugger/property_debugger/tweaker_type.h"
#include "application/setups/debugger/property_debugger/widgets/asset_path_chooser.h"

struct pathed_asset_widget {
	all_viewables_defs& defs;
	const augs::path_type& project_path;
	const debugger_command_input in;

	template <class T>
	static constexpr bool handles =
		is_pathed_asset<T>
	;

	template <class T>
	static constexpr bool handles_prologue = false;

	template <class T>
	auto describe_changed(
		const std::string& formatted_label,
		const T& to
	) const {
		static_assert(handles<T>);

		return typesafe_sprintf("Set %x to %x", formatted_label, augs::filename_first(get_viewable_pool<T>(defs)[to].get_source_path().path));
	}

	template <class T>
	std::optional<tweaker_type> handle(const std::string& identity_label, T& object) const {
		static_assert(handles<T>);

		bool changed = false;

		const auto& definitions = get_viewable_pool<T>(defs);

		auto on_choice = [&](const maybe_official_path<T>& chosen_path) {
			object = ::get_id_or_import(chosen_path, project_path, definitions, in);
			changed = true;
		};
		
		thread_local asset_path_chooser<T> chooser;

		/* Definitions might get altered so we need to keep a copy */
		const auto found_def = mapped_or_nullptr(definitions, object);

		using P = remove_cref<decltype(found_def->get_source_path())>;

		const auto current_source = found_def ? P(found_def->get_source_path()) : P();

		chooser.perform(identity_label, current_source, project_path, on_choice, true_returner());

		if (changed) {
			return tweaker_type::DISCRETE;
		}

		return std::nullopt;
	}
};
