#pragma once
#include <optional>
#include <future>
#include <vector>

#include "augs/image/font.h"
#include "augs/texture_atlas/atlas_profiler.h"
#include "augs/graphics/renderer.h"
#include "view/viewables/streaming/viewables_streaming_profiler.h"
#include "view/viewables/all_viewables_defs.h"
#include "view/viewables/loaded_sounds_map.h"

#include "view/viewables/atlas_distributions.h"
#include "view/viewables/regeneration/content_regeneration_settings.h"
#include "view/viewables/avatars_in_atlas_map.h"
#include "view/viewables/ad_hoc_in_atlas_map.h"
#include "augs/texture_atlas/loaded_images_vector.h"
#include "view/viewables/regeneration/atlas_progress_structs.h"
#include "augs/graphics/frame_num_type.h"

#include "augs/filesystem/file_time_type.h"

class sound_system;

namespace augs {
	class audio_command_buffers;
}

struct viewables_load_input {
	const augs::frame_num_type current_frame;
	const all_viewables_defs& new_defs;
	const necessary_image_definitions_map& necessary_image_definitions;
	const all_gui_fonts_inputs& gui_fonts;
	const content_regeneration_settings settings;
	const augs::path_type& unofficial_content_dir;

	augs::renderer& renderer;
	const unsigned max_atlas_size;
	std::optional<arena_player_metas>& new_player_metas;
	std::optional<ad_hoc_atlas_subjects> ad_hoc_subjects;
};

struct viewables_finalize_input {
	augs::audio_command_buffers& audio_buffers;
	const augs::frame_num_type current_frame;
	const bool measure_atlas_upload;
	augs::renderer& renderer;

	sound_system& sounds;
};

template <class output_type>
struct texture_in_progress {
	std::vector<rgba> pbo_fallback;
	std::future<output_type> future_output;
	std::optional<augs::frame_num_type> submitted_when;
	augs::graphics::texture texture = augs::image::white_pixel();

	decltype(output_type::atlas_entries) in_atlas;

	bool work_slot_free(const augs::frame_num_type current_frame) const {
		return !future_output.valid() && augs::has_completed(current_frame, submitted_when);
	}

	template <class T>
	void submit_work(T&& callable) {
		pbo_fallback.clear();
		future_output = launch_async(std::forward<T>(callable));
	}

	void finalize_tasks_if_any() {
		if (future_output.valid()) {
			future_output.get();
		}
	}

	void finalize_load(augs::renderer&);
};

class viewables_streaming {
	std::vector<rgba> general_atlas_pbo_fallback;

	all_loaded_gui_fonts loaded_gui_fonts;

	image_definitions_map future_image_definitions;
	all_gui_fonts_inputs future_gui_fonts;

	std::future<general_atlas_output> future_general_atlas;

	all_viewables_defs now_loaded_viewables_defs;
	all_gui_fonts_inputs now_loaded_gui_font_defs;

	sound_definitions_map future_sound_definitions;
	std::vector<std::pair<assets::sound_id, augs::sound_buffer_loading_input>> sound_requests;
	std::future<std::vector<std::optional<augs::sound_buffer>>> future_loaded_buffers;

	std::vector<augs::file_time_type> image_write_times;
	std::vector<augs::file_time_type> sound_write_times;

	std::optional<atlas_progress_structs> general_atlas_progress;
	std::optional<augs::frame_num_type> general_atlas_submitted_when;

	bool rescan_for_modified_images = false;
	bool rescan_for_modified_sounds = false;

public:
	augs::graphics::texture avatar_preview_tex = augs::image::white_pixel();
	augs::graphics::texture general_atlas = augs::image::white_pixel();

	texture_in_progress<avatar_atlas_output> avatars;
	texture_in_progress<ad_hoc_atlas_output> ad_hoc;

	std::optional<ad_hoc_atlas_subjects> last_requested_ad_hoc_atlas;
	std::optional<arena_player_metas> last_requested_player_metas;

	viewables_streaming_profiler performance;

	loaded_sounds_map loaded_sounds;
	images_in_atlas_map images_in_atlas;
	necessary_images_in_atlas_map necessary_images_in_atlas;

	atlas_profiler general_atlas_performance;
	atlas_profiler neon_map_atlas_performance;

	viewables_streaming() = default;
	~viewables_streaming();

	void load_all(viewables_load_input);
	void finalize_load(viewables_finalize_input);

	auto& get_loaded_gui_fonts() {
		return loaded_gui_fonts;
	}

	void finalize_pending_tasks();

	bool finished_generating_atlas() const;
	void display_loading_progress() const;

	void request_rescan();
};
