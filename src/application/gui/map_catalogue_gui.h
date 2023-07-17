#pragma once
#include <future>
#include <unordered_set>
#include "augs/network/network_types.h"
#include "augs/misc/imgui/standard_window_mixin.h"
#include "view/viewables/ad_hoc_in_atlas_map.h"
#include "view/viewables/ad_hoc_atlas_subject.h"
#include "view/necessary_resources.h"

namespace augs {
	class window;
}

struct map_catalogue_entry {
	// GEN INTROSPECTOR struct map_catalogue_entry
	std::string name;
	std::string author;
	std::string short_description;
	std::string version_timestamp;
	// END GEN INTROSPECTOR

	ad_hoc_entry_id miniature_id = 0;
	std::optional<std::string> version_on_disk;
};

struct ad_hoc_in_atlas_map;

struct map_catalogue_input {
	address_string_type& external_arena_files_provider;
	const ad_hoc_in_atlas_map& ad_hoc_atlas;
	const necessary_images_in_atlas_map& necessary_images;
	augs::window& window;
};

struct map_catalogue_gui_internal;

class map_catalogue_gui_state : public standard_window_mixin<map_catalogue_gui_state> {
	std::vector<map_catalogue_entry> map_list;
	std::future<std::vector<map_catalogue_entry>> future_response;
	std::future<void> future_downloaded_miniatures;

	std::atomic<uint32_t> has_next_miniature = 0;
	std::vector<ad_hoc_atlas_subject> completed_miniatures;

	std::unique_ptr<map_catalogue_gui_internal> data;
	std::vector<ad_hoc_atlas_subject> last_miniatures;
	bool mark_rebuild_miniatures = false;

	std::string last_error;
	bool refreshed_once = false;
	bool focus_on_filter_once = true;

	int sort_by_column = 1;
	bool ascending = false;

	std::set<std::string> selected_arenas;
	const map_catalogue_entry* last_selected = nullptr;

	ImGuiTextFilter filter;

	void request_miniatures(map_catalogue_input);

	void perform_list(map_catalogue_input);

	bool refresh_in_progress() const;
	bool list_refresh_in_progress() const;
	void refresh(map_catalogue_input);

public:

	using base = standard_window_mixin<map_catalogue_gui_state>;
	map_catalogue_gui_state(const std::string& title);
	~map_catalogue_gui_state();

	bool perform(map_catalogue_input);
	void rebuild_miniatures();

	std::optional<std::vector<ad_hoc_atlas_subject>> get_new_ad_hoc_images();
};
