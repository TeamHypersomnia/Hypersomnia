#pragma once
#include <future>
#include <unordered_set>
#include "augs/network/network_types.h"
#include "augs/string/parse_url.h"
#include "augs/misc/imgui/simple_popup.h"

using ad_hoc_entry_id = uint32_t;

struct map_catalogue_entry {
	// GEN INTROSPECTOR struct map_catalogue_entry
	std::string name;
	std::string author;
	std::string short_description;
	version_timestamp_string version_timestamp;
	// END GEN INTROSPECTOR

	mutable std::optional<std::string> version_on_disk;

	mutable ad_hoc_entry_id miniature_id = 0;
	mutable std::size_t last_displayed_index = 0;
	mutable bool passed_filter = true;

	enum class state {
		NOT_FOUND,
		ON_DISK,
		UPDATE_AVAILABLE
	};

	state get_state() const;
};

struct headless_map_catalogue_input {
	address_string_type external_arena_files_provider;
};

enum class headless_catalogue_result {
	IN_PROGRESS,
	LIST_REFRESH_COMPLETE
};

struct multi_arena_synchronizer_internal;

struct arena_downloading_session;
struct arena_synchronizer_input_entry {
	std::string name;
	version_timestamp_string version;

	mutable std::size_t idx = 0;

	bool operator<(const arena_synchronizer_input_entry& b) const {
		return idx < b.idx;
	}
};

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
	float get_current_map_progress() const;

	std::optional<std::string> get_current_map_name() const;
	std::size_t get_current_map_index() const { return current_map; }
	std::size_t get_total_maps() const { return input.size(); }

	template <class F>
	void for_each_with_progress(F callback) const;

	/* Flow control */
	void advance();
	bool finished() const;
	std::optional<std::string> get_error() const;
};

class headless_map_catalogue {
	std::vector<map_catalogue_entry> map_list;
	std::future<std::vector<map_catalogue_entry>> future_response;

	std::string list_catalogue_error;

	bool refreshed_once = false;
	std::optional<multi_arena_synchronizer> downloading;

	std::unordered_set<std::string> official_names;

	bool should_rescan = false;

	void rescan_official_arenas();
	void rescan_versions_on_disk();

public:
	std::optional<simple_popup> download_failed_popup;

	headless_map_catalogue();
	~headless_map_catalogue();

	headless_catalogue_result advance(headless_map_catalogue_input);

	template <class... Args>
	void launch_download(Args&&... args) {
		downloading.emplace(std::forward<Args>(args)...);
	}

	arena_synchronizer_input launch_download_all(const address_string_type&);

	bool finalize_download();

	void request_rescan();
	void request_refresh();

	const std::optional<multi_arena_synchronizer>& get_downloading() const { return downloading; }

	void refresh(address_string_type);

	const auto& get_map_list() const { return map_list; }
	const auto& get_list_catalogue_error() const { return list_catalogue_error; }

	bool list_refresh_in_progress() const;
};
