#pragma once
#include "augs/filesystem/path.h"
#include "game/assets/ids/asset_ids.h"
#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/commands/editor_command_structs.h"
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

template <class T>
struct is_create_asset_id_command : std::false_type {};

template <class T>
struct is_create_asset_id_command<create_asset_id_command<T>> : std::true_type {};

template <class T>
constexpr bool is_create_asset_id_command_v = is_create_asset_id_command<T>::value;

