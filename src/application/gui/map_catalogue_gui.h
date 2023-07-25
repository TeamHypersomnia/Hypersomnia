#pragma once
#include <future>
#include <unordered_set>
#include "augs/network/network_types.h"
#include "augs/misc/imgui/standard_window_mixin.h"
#include "view/viewables/ad_hoc_in_atlas_map.h"
#include "view/viewables/ad_hoc_atlas_subject.h"
#include "view/necessary_resources.h"
#include "augs/misc/imgui/simple_popup.h"

namespace augs {
	class window;
}

struct map_catalogue_entry {
	// GEN INTROSPECTOR struct map_catalogue_entry
	std::string name;
	std::string author;
	std::string short_description;
	version_timestamp_string version_timestamp;
	// END GEN INTROSPECTOR

	ad_hoc_entry_id miniature_id = 0;
	std::optional<std::string> version_on_disk;
	mutable std::size_t last_displayed_index = 0;
	mutable bool passed_filter = true;

	enum class state {
		NOT_FOUND,
		ON_DISK,
		UPDATE_AVAILABLE
	};

	state get_state() const;
};

struct ad_hoc_in_atlas_map;

struct map_catalogue_input {
	address_string_type& external_arena_files_provider;
	const ad_hoc_in_atlas_map& ad_hoc_atlas;
	const necessary_images_in_atlas_map& necessary_images;
	augs::window& window;
};

struct multi_arena_synchronizer_internal;

struct parsed_url;

struct arena_synchronizer_input_entry {
	std::string name;
	version_timestamp_string version;

	mutable std::size_t idx = 0;

	bool operator<(const arena_synchronizer_input_entry& b) const {
		return idx < b.idx;
	}
};

struct arena_downloading_session;
using arena_synchronizer_input = std::vector<arena_synchronizer_input_entry>;

class multi_arena_synchronizer {
	arena_synchronizer_input input;
	std::unique_ptr<multi_arena_synchronizer_internal> data;
	std::size_t current_map = 0;

	void init_next_session();

	arena_downloading_session& session();
	const arena_downloading_session& session() const;

	float get_current_file_percent_complete() const;

	std::optional<std::string> last_error;

public:
	multi_arena_synchronizer(
		const arena_synchronizer_input& arena_names,
		const parsed_url& parent_folder_url
	);

	~multi_arena_synchronizer();

	const auto& get_input() const { return input; }

	float get_total_progress() const;

	template <class F>
	void for_each_with_progress(F callback) const;

	/* Flow control */
	void advance();
	bool finished() const;
	std::optional<std::string> get_error() const;
};

class map_catalogue_gui_state : public standard_window_mixin<map_catalogue_gui_state> {
	std::vector<map_catalogue_entry> map_list;
	std::future<std::vector<map_catalogue_entry>> future_response;
	std::future<void> future_downloaded_miniatures;

	std::atomic<uint32_t> has_next_miniature = 0;
	std::vector<ad_hoc_atlas_subject> completed_miniatures;

	std::vector<ad_hoc_atlas_subject> last_miniatures;
	bool mark_rebuild_miniatures = false;

	std::string last_error;
	bool refreshed_once = false;
	bool focus_on_filter_once = true;

	int sort_by_column = 1;
	bool ascending = false;

	std::optional<multi_arena_synchronizer> downloading;

	std::unordered_set<std::string> official_names;

	std::unordered_set<const map_catalogue_entry*> selected_arenas;

	const map_catalogue_entry* last_selected = nullptr;

	ImGuiTextFilter filter;

	void request_miniatures(map_catalogue_input);

	void perform_list(map_catalogue_input);

	bool refresh_in_progress() const;
	bool list_refresh_in_progress() const;
	void refresh(map_catalogue_input);

	void rescan_official_arenas();

	static void rescan_versions_on_disk(std::vector<map_catalogue_entry>&);

	bool launch_download_on_last_selected = false;
	std::optional<simple_popup> download_failed_popup;

	bool should_rescan = false;

public:

	using base = standard_window_mixin<map_catalogue_gui_state>;
	map_catalogue_gui_state(const std::string& title);
	~map_catalogue_gui_state();

	bool perform(map_catalogue_input);
	void rebuild_miniatures();

	void rescan_versions_on_disk();
	void request_rescan();

	std::optional<std::vector<ad_hoc_atlas_subject>> get_new_ad_hoc_images();

	bool is_downloading() const { return downloading.has_value(); }

	std::optional<std::string> open_host_window;
};
