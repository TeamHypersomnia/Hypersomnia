#pragma once
#include <unordered_set>
#include "augs/misc/future.h"
#include "augs/network/network_types.h"
#include "augs/misc/imgui/standard_window_mixin.h"
#include "view/viewables/ad_hoc_in_atlas_map.h"
#include "view/viewables/ad_hoc_atlas_subject.h"
#include "view/necessary_resources.h"
#include "augs/misc/imgui/simple_popup.h"
#include "application/gui/headless_map_catalogue.h"

#include "application/setups/client/https_file_downloader.h"

namespace augs {
	class window;
}

struct ad_hoc_in_atlas_map;

struct map_catalogue_input {
	address_string_type& external_arena_files_provider;
	const ad_hoc_in_atlas_map& ad_hoc_atlas;
	const necessary_images_in_atlas_map& necessary_images;
	augs::window& window;
	bool streamer_mode;

	auto make_headless() const {
		return headless_map_catalogue_input { external_arena_files_provider };
	}
};

struct parsed_url;

class map_catalogue_gui_state : public standard_window_mixin<map_catalogue_gui_state> {
	headless_map_catalogue headless;

	std::optional<https_file_downloader> miniature_downloader;
	std::vector<ad_hoc_atlas_subject> last_miniatures;

	bool mark_rebuild_miniatures = false;

	bool focus_on_filter_once = true;

	int sort_by_column = 1;
	bool ascending = false;

	std::unordered_set<std::string> selected_arenas;

	std::string last_selected;

	ImGuiTextFilter filter;

	void request_miniatures(map_catalogue_input);

	void perform_list(map_catalogue_input);

	bool refresh_in_progress() const;

	bool launch_download_on_last_selected = false;
	bool only_playable = true;


public:

	using base = standard_window_mixin<map_catalogue_gui_state>;
	map_catalogue_gui_state(const std::string& title);
	~map_catalogue_gui_state();

	bool perform(map_catalogue_input);
	void rebuild_miniatures();

	void refresh(address_string_type);
	void request_rescan();
	void request_refresh();

	std::optional<std::vector<ad_hoc_atlas_subject>> get_new_ad_hoc_images();

	const auto& get_downloading() const { return headless.get_downloading(); };
	bool is_downloading() const { return get_downloading().has_value(); }

	std::optional<std::string> open_host_server_window;

	const map_catalogue_entry* find_entry_by(const std::string&);
};
