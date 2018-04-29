#pragma once
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/templates/identity_templates.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "view/viewables/all_viewables_defs.h"

#include "augs/misc/imgui/path_tree_structs.h"
#include "application/setups/editor/gui/asset_browser_settings.h"
#include "application/setups/editor/property_editor/tweaker_type.h"
#include "application/setups/editor/property_editor/assets/asset_path_chooser.h"

struct asset_sane_default_provider {
	all_viewables_defs& defs;

	template <class T>
	auto construct() const {
		if constexpr(std::is_same_v<T, invariants::sprite>) {
			auto& definitions = defs.image_definitions;
			invariants::sprite t;
			t.tex = definitions.get_nth_id(0);
			return t;
		}

		return T();
	}
};

struct asset_control_provider {
	all_viewables_defs& defs;
	const augs::path_type& project_path;
	editor_command_input in;

	template <class T>
	static constexpr bool handles = 
		is_one_of_v<T, assets::image_id, assets::sound_id>
	;

	template <class T>
	auto describe_changed(
		const std::string& formatted_label,
		const T to
	) const {
		return typesafe_sprintf("Set %x to %x", formatted_label, augs::to_display(get_viewable_pool<T>(defs)[to].get_source_path().path));
	}

	template <class T>
	std::optional<tweaker_type> handle(const std::string& identity_label, T& object) const {
		bool changed = false;

		if constexpr(handles<T>) {
			auto& definitions = get_viewable_pool<T>(defs);

			auto on_choice = [&](const auto& chosen_path) {
				changed = true;

				if (const auto asset_id = ::find_asset_id_by_path(chosen_path, definitions)) {
					object = *asset_id;
				}
				else {
					auto& history = in.folder.history;

					{
						create_asset_id_command<T> cmd;
						cmd.use_path = chosen_path;
						history.execute_new(std::move(cmd), in);
					}

					const auto* const last_addr = std::addressof(history.last_command());
					const auto* const cmd = std::get_if<create_asset_id_command<T>>(last_addr);

					object = cmd->get_allocated_id();
				}
			};
			
			const auto& current_source  = definitions[object].get_source_path();

			thread_local asset_path_chooser<T> chooser;

			chooser.perform(identity_label, current_source, project_path, on_choice, true_returner());
		}
		else {
			static_assert(!handles<T>, "Incomplete implementation!");
		}

		if (changed) {
			return tweaker_type::DISCRETE;
		}

		return std::nullopt;
	}
};
