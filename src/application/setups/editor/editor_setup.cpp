#include "game/cosmos/logic_step.h"
#include "game/organization/all_messages_includes.h"
#include "augs/misc/pool/pool_allocate.h"
#include "augs/misc/secure_hash.h"

#include "view/viewables/viewables_loading_type.h"

#include "application/config_lua_table.h"
#include "application/setups/editor/editor_setup.h"
#include "application/setups/editor/editor_paths.h"
#include "application/setups/editor/project/editor_project_readwrite.h"

#include "augs/filesystem/directory.h"
#include "augs/readwrite/byte_readwrite.h"
#include "augs/readwrite/byte_file.h"
#include "augs/log.h"

#include "augs/misc/time_utils.h"
#include "application/setups/editor/resources/editor_sprite_resource.h"
#include "application/setups/editor/resources/editor_sound_resource.h"

editor_setup::editor_setup(const augs::path_type& project_path) : paths(project_path) {
	LOG("Loading editor project at: %x", project_path);
	project = editor_project_readwrite::read_project_json(paths.project_json);

	load_gui_state();
	open_default_windows();

	on_window_activate();
}

editor_setup::~editor_setup() {
	save_gui_state();
}

void editor_setup::open_default_windows() {
	gui.inspector.open();
	gui.layers.open();
	gui.filesystem.open();
}

bool editor_setup::handle_input_before_imgui(
	handle_input_before_imgui_input in
) {
	using namespace augs::event;

	if (in.e.msg == message::activate) {
		on_window_activate();
	}

	if (in.e.msg == message::deactivate) {
		force_autosave_now();
	}

	return false;
}

bool editor_setup::handle_input_before_game(
	handle_input_before_game_input
) {
	return false;
}

void editor_setup::customize_for_viewing(config_lua_table& config) const {
	config.window.name = typesafe_sprintf("Hypersomnia Editor - %x", project.meta.name);
}

std::optional<ad_hoc_atlas_subjects> editor_setup::get_new_ad_hoc_images() {
	if (rebuild_ad_hoc_atlas) {
		rebuild_ad_hoc_atlas = false;

		ad_hoc_atlas_subjects new_subjects;
		files.fill_thumbnail_entries(paths.project_folder, new_subjects);

		if (new_subjects == last_ad_hoc_subjects) {
			return std::nullopt;
		}

		last_ad_hoc_subjects = new_subjects;
		return new_subjects;
	}

	return std::nullopt;
}

void editor_setup::on_window_activate() {
	rebuild_filesystem();
}

void editor_setup::rebuild_filesystem() {
	files.rebuild_from(paths.project_folder);
	gui.filesystem.clear_pointers();
	rebuild_ad_hoc_atlas = true;

	const auto changes = rebuild_pathed_resources();

	if (changes.any()) {
		simple_popup changes_popup;

		changes_popup.title = "Detected changes in filesystem";
		std::string summary;

		const int redirs = changes.redirects.size();
		const int missing = changes.missing.size();

		if (missing > 0) {
			summary += typesafe_sprintf("%x files are missing!\n", missing);
		}

		if (redirs > 0) {
			summary += typesafe_sprintf("%x files have been automatically redirected.\n", redirs);
		}

		std::string details;

		for (auto& c : changes.missing) {
			details += typesafe_sprintf("Missing: %x\n", c.string());
		}

		for (auto& r : changes.redirects) {
			details += typesafe_sprintf("Redirected: %x -> %x\n", r.first.string(), r.second.string());
		}

		changes_popup.title = "Detected changes in filesystem";
		changes_popup.message = summary;
		changes_popup.details = details;

		ok_only_popup = changes_popup;
	}
}

editor_paths_changed_report editor_setup::rebuild_pathed_resources() {
	editor_paths_changed_report changes;

	/*
		Before allocating a resource, we want to first check if one exists with this content hash,
		and with own path no longer existing. Only then do we redirect.

		Otherwise we could end up with duplicate resources.
	*/

	auto handle_pool = [&]<typename P>(P& pool, const editor_filesystem_node_type type) {
		using R = editor_pathed_resource;

		std::unordered_map<std::string,     R*> resource_by_hash;
		std::unordered_map<augs::path_type, R*> resource_by_path;

		auto find_resource = [&](auto& in, const auto& by) -> R* {
			if (auto found = mapped_or_nullptr(in, by)) {
				return *found;
			}

			return nullptr;
		};

		for (auto& entry : pool) {
			auto& r = entry.external_file;

			resource_by_hash[r.content_hash] = std::addressof(r);
			resource_by_path[r.path]         = std::addressof(r);
		}

		auto add_if_new = [&](editor_filesystem_node& file) {
			if (file.type != type) {
				return;
			}

			const auto path = file.get_path();
			const auto full_path = paths.project_folder / path;

			if (auto found_resource = find_resource(resource_by_path, path)) {
				found_resource->maybe_rehash(full_path, file.last_write_time);
			}
			else {
				std::string new_resource_hash;

				try {
					new_resource_hash = augs::secure_hash(augs::file_to_bytes(full_path));
				}
				catch (...) {

				}

				if (new_resource_hash.empty()) {
					LOG("WARNING! Couldn't get a hash from %x", full_path);
					return;
				}

				auto moved_resource = find_resource(resource_by_hash, new_resource_hash);

				if (moved_resource && !augs::exists(moved_resource->path)) {
					const auto& new_path = path;

					changes.redirects.emplace_back(moved_resource->path, new_path);

					moved_resource->path = new_path;
					moved_resource->set_hash_stamp(file.last_write_time);
				}
				else {
					auto it = pool.allocate(editor_pathed_resource(path, new_resource_hash, file.last_write_time));
					auto new_id = editor_resource_id();

					new_id.is_official = false;
					new_id.raw = it.key;
					new_id.type_id = editor_resource_type_id::template of<typename P::value_type>();

					file.associated_resource = new_id;
				}
			}
		};

		files.root.for_each_file_recursive(add_if_new);

		for (auto& entry : pool) {
			auto& r = entry.external_file;

			if (!augs::exists(r.path)) {
				/* 
					If it's still missing after redirect, 
					then it is indeed missing.
				*/

				changes.missing.emplace_back(r.path);
			}
		}
	};

	auto& sprite_pool = project.resources.pools.template get_for<editor_sprite_resource>();
	auto& sound_pool  = project.resources.pools.template get_for<editor_sound_resource>();

	handle_pool(sprite_pool, editor_filesystem_node_type::IMAGE);
	handle_pool(sound_pool, editor_filesystem_node_type::SOUND);

	/*
		Corner cases:

		If someone deletes a resource file (e.g. .png or .ogg), it's still in memory and its parameters will be written to the json file.
		If someone moves a file AND modifies it too, it will simply be considered missing.

		Every time you activate the window the editor will try redirecting the deleted files.
	*/

	return changes;
}

void editor_setup::force_autosave_now() {

}

void editor_setup::load_gui_state() {
	/*
		To be decided what to do about it.
		Generally ImGui will save the important layouts on its own.
		The identifiers (e.g. currently inspected object id) might become out of date after reloading from json.
	*/
}

void editor_setup::save_gui_state() {
	augs::save_as_text(get_editor_last_project_path(), paths.project_folder.string());
}

void editor_pathed_resource::maybe_rehash(const augs::path_type& full_path, const augs::file_time_type& fresh_stamp) {
	const auto fresh_stamp_utc = fresh_stamp;

	if (stamp_when_hashed == fresh_stamp_utc && content_hash.size() > 0) {
		return;
	}

	try {
		content_hash = augs::secure_hash(augs::file_to_bytes(full_path));
		stamp_when_hashed = fresh_stamp_utc;
	}
	catch (...) {
		content_hash = "";
	}
}

editor_pathed_resource::editor_pathed_resource(
	const augs::path_type& path, 
	const std::string& content_hash,
	const augs::file_time_type& stamp
) : 
	path(path),
	content_hash(content_hash)
{
	set_hash_stamp(stamp);
}

void editor_pathed_resource::set_hash_stamp(const augs::file_time_type& stamp) {
	stamp_when_hashed = stamp;
}
