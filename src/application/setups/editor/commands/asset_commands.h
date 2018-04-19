#pragma once
#include "augs/filesystem/path.h"
#include "game/assets/ids/asset_ids.h"
#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/commands/editor_command_structs.h"
#include "application/setups/editor/commands/change_property_command.h"
#include "application/setups/editor/property_editor/property_editor_structs.h"
#include "view/viewables/all_viewables_declarations.h"

namespace augs {
	struct introspection_access;
}

template <class id_type>
struct create_asset_id_command {
	// GEN INTROSPECTOR struct create_asset_id_command class id_type
	editor_command_common common;
private:
	friend augs::introspection_access;
	id_type allocated_id;
public:
	std::string use_path;
	// END GEN INTROSPECTOR

	auto get_allocated_id() const {
		return allocated_id;
	}

	std::string describe() const;
	void redo(const editor_command_input in);
	void undo(const editor_command_input in);
};

template <class id_type>
struct forget_asset_id_command {
	// GEN INTROSPECTOR struct forget_asset_id_command class id_type
	editor_command_common common;

	id_type forgotten_id;
	std::string built_description;
private:
	friend augs::introspection_access;
	typename id_type::undo_free_type undo_free_input;
	std::vector<std::byte> forgotten_content;
public:
	// END GEN INTROSPECTOR

	std::string describe() const;
	void redo(const editor_command_input in);
	void undo(const editor_command_input in);
};

template <class id_type, bool of_meta = false>
struct change_asset_property_command : change_property_command<change_asset_property_command<id_type, of_meta>> {
	using introspect_base = change_property_command<change_asset_property_command>;

	// GEN INTROSPECTOR struct change_asset_property_command class id_type bool of_meta
	editor_command_common common;

	field_address field;
	std::vector<id_type> affected_assets;
	// END GEN INTROSPECTOR

	auto count_affected() const {
		return affected_assets.size();
	}

	template <class T, class F>
	void access_each_property(
		T in,
		F&& callback
	) const {
		auto& viewables = in.folder.work->viewables;

		if constexpr(std::is_same_v<id_type, assets::image_id>) {
			if constexpr(of_meta) {
				auto& metas = viewables.image_metas;

				for (const auto& id : affected_assets) {
					on_field_address(
						metas[id],
						field,
						[&](auto& resolved_field) {
							callback(resolved_field);
						}
					);
				}
			}
			else {
				auto& loadables = viewables.image_loadables;

				for (const auto& id : affected_assets) {
					on_field_address(
						loadables[id],
						field,
						[&](auto& resolved_field) {
							callback(resolved_field);
						}
					);
				}
			}
		}
		else {
			static_assert(always_false_v<T>, "Unsupported id type.");
		}
	}
};

template <class T>
struct is_create_asset_id_command : std::false_type {};

template <class T>
struct is_create_asset_id_command<create_asset_id_command<T>> : std::true_type {};

template <class T>
constexpr bool is_create_asset_id_command_v = is_create_asset_id_command<T>::value;

