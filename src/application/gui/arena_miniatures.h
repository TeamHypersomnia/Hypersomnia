#pragma once
#include <string>
#include <optional>
#include <unordered_map>
#include "augs/filesystem/path.h"
#include "augs/filesystem/file_time_type.h"
#include "augs/templates/container_templates.h"
#include "augs/texture_atlas/atlas_entry.h"
#include "view/viewables/ad_hoc_in_atlas_map.h"
#include "view/viewables/ad_hoc_atlas_subject.h"

/*
	Small miniatures of arenas shown left of the arena names in the arena choosers -
	an ad hoc atlas of their own (viewables_streaming::arena_miniatures, imgui_atlas_type::ARENA_MINIATURES),
	packed in the background like the project selector's, with the miniatures scaled down.

	A chooser asks for its arena by name every frame it is drawn.
	A newly asked arena gets its miniature.png looked up like a server looks for an arena: official, user projects, downloaded -
	and the atlas is packed again with it. When a chooser opens, the miniatures changed on disk
	(saved in the editor, downloaded later) are packed again too.
*/

class arena_miniatures {
	struct known_arena {
		ad_hoc_entry_id id = 0;
		augs::path_type path;

		/*
			std::nullopt for an arena without a miniature on disk.
		*/

		std::optional<augs::file_time_type> write_time;
	};

	std::unordered_map<std::string, known_arena> known;
	ad_hoc_entry_id next_id = 1;
	bool subjects_changed = false;

	const ad_hoc_in_atlas_map* in_atlas = nullptr;

public:
	const augs::atlas_entry* request(const std::string& arena_name);
	void request_again_changed_on_disk();

	void set_in_atlas(const ad_hoc_in_atlas_map&);
	std::optional<ad_hoc_atlas_subjects> take_new_subjects();
};

arena_miniatures& get_arena_miniatures();
