#include "augs/log.h"
#include "augs/misc/timing/timer.h"
#include "augs/filesystem/file.h"
#include "augs/misc/imgui/imgui_game_image.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "game/assets/animation_math.h"
#include "view/viewables/images_in_atlas_map.h"
#include "application/setups/debugger/property_debugger/widgets/frames_prologue_widget.h"
#include "application/setups/debugger/detail/field_address.h"

#include "application/setups/debugger/detail/get_id_or_import.h"
#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/byte_readwrite.h"
#include "augs/readwrite/to_bytes.h"

bool frames_prologue_widget::handle_prologue(const std::string&, plain_animation_frames_type&) const {
	using namespace augs::imgui;

	using frames_type = plain_animation_frames_type;
	using frame_type = typename frames_type::value_type;

	using change_prop_command = change_asset_property_command<animation_id_type>;

	const auto& logicals = cmd_in.get_logical_assets();
	const auto& defs = cmd_in.get_viewable_defs();
	const auto& image_defs = defs.image_definitions;

	if (preview_animations) {
		thread_local augs::timer anim_timer;

		const auto& anim = *logicals.find(id);
		const auto total_duration = static_cast<double>(::calc_total_duration(anim.frames));
		const auto total_time = anim_timer.get<std::chrono::milliseconds>();
		const auto considered_time = std::fmod(total_time, total_duration);
		const auto current_frame = ::calc_current_frame(anim, considered_time);

		const auto& entry = game_atlas.find_or(current_frame->image_id);

		const auto is = vec2i(entry.get_original_size());

		const auto max_aabb = ::get_max_frame_size(anim.frames, game_atlas);

		game_image(entry.diffuse, is, white, max_aabb / 2 - is / 2);
		invisible_button("###animpreview", max_aabb);
	}

	bool has_parent = false;

	/* 
		TODO: Probably let history have push/pop commands parent?
		So that we don't have funny lambdas for has_parent.
	*/

	auto get_has_parent = [&has_parent]() {
		const bool hp = has_parent;
		has_parent = true;
		return hp;
	};

	auto on_current_or_ticked = [&](auto callback) {
		if (!is_current_ticked) {
			callback(id);
		}
		else {
			for_each_in(ticked_ids, callback);
		}
	};

	auto post_new_frames = [&](
		const auto id,
		const auto& new_frames,
		auto&& description
	) {
		change_prop_command cmd;

		cmd.common.has_parent = get_has_parent();
		cmd.affected_assets = { id };
		cmd.property_id.field = MACRO_MAKE_ASSET_FIELD_ADDRESS(plain_animation, frames);
		augs::assign_bytes(cmd.value_after_change, new_frames);
		cmd.built_description = std::forward<decltype(description)>(description);

		post_debugger_command(cmd_in, std::move(cmd));
	};

	if (ImGui::Button("From file sequence")) {
		auto import_frames = [&](const auto& id) {
			const auto& source_animation = *logicals.find(id);
			const auto& frames = source_animation.frames;

			const auto& first_image_def = image_defs[frames[0].image_id];
			const auto& first_source_path = first_image_def.get_source_path();
			const auto first_path_no_ext = augs::path_type(first_source_path.path).replace_extension("");

			frames_type new_frames;
			new_frames.push_back(frames[0]);

			if (const auto num = get_trailing_number(first_path_no_ext.string())) {
				auto first_path_no_num = first_path_no_ext.string();
				cut_trailing_number(first_path_no_num);

				const auto ext = first_source_path.path.extension();

				for (auto i = *num + 1; ; ++i) {
					auto next_source_path = first_source_path;
					next_source_path.path = typesafe_sprintf("%x%x%x", first_path_no_num, i, ext);

					if (augs::exists(next_source_path.resolve(project_dir))) {
						frame_type frame;

						{
							const auto corresponding_frame_idx = new_frames.size();

							if (corresponding_frame_idx < frames.size()) {
								/* Keep other properties, e.g. frame duration */
								frame = frames[corresponding_frame_idx];
							}
						}

						frame.image_id = ::get_id_or_import(
							next_source_path,
							project_dir,
							image_defs,
							cmd_in
						);

						new_frames.push_back(frame);

						if (container_full(new_frames)) {
							LOG("WARNING: While importing, reached the maximum animation frame count of %x.", new_frames.size());
							break;
						}
					}
					else {
						break;
					}
				}
			}

			auto description = typesafe_sprintf("Read a sequence of %x frames starting from %x", new_frames.size(), first_source_path.path);
			post_new_frames(id, new_frames, description);
		};

		on_current_or_ticked(import_frames);
	}

	if (ImGui::Button("Reverse")) {
		auto reverse_frames = [&](const auto& id) {
			const auto& source_animation = *logicals.find(id);
			auto new_frames = source_animation.frames;

			auto description = typesafe_sprintf("Reversed frames in %x", get_displayed_name(source_animation, image_defs));
			post_new_frames(id, reverse_range(new_frames), description);
		};

		on_current_or_ticked(reverse_frames);
	}

	ImGui::SameLine();

	if (ImGui::Button("Ping-pong")) {
		auto reverse_frames = [&](const auto& id) {
			const auto& source_animation = *logicals.find(id);
			auto new_frames = source_animation.frames;
			ping_pong_range(new_frames);

			auto description = typesafe_sprintf("Ping-ponged frames in %x", get_displayed_name(source_animation, image_defs));
			post_new_frames(id, new_frames, description);
		};

		on_current_or_ticked(reverse_frames);
	}

	using duration_type = decltype(frame_type::duration_milliseconds);

	std::optional<duration_type> constant_rate;
	bool abrt = false;

	auto check_if_constant = [&](const auto& id) {
		if (abrt) { return; }

		const auto& source_animation = *logicals.find(id);

		for (auto& f : source_animation.frames) {
			if (!constant_rate) {
				constant_rate = f.duration_milliseconds;
			}
			else if (constant_rate != f.duration_milliseconds) {
				constant_rate = std::nullopt;
				abrt = true;
				return;
			}
		}
	};

	on_current_or_ticked(check_if_constant);

	const auto fmt = constant_rate ? "%.0f ms" : "Set...";

	{
		text("Constant framerate:");

		ImGui::SameLine();

		auto sw = scoped_item_width(-1);

		if (!constant_rate) {
			constant_rate = 0.f;
		}

		if (drag("###constantframerate", *constant_rate, 1.f, 1.f, 1000.f, fmt)) {
			const bool new_cmd = prop_in.state.tweaked_widget_changed();

			if (!new_cmd) {
				cmd_in.get_history().undo(cmd_in);
			}

			auto set_constant_rate = [&](const auto id) {
				const auto& source_animation = *logicals.find(id);
				auto new_frames = source_animation.frames;

				for (auto& f : new_frames) {
					f.duration_milliseconds = *constant_rate;
				}

				auto description = typesafe_sprintf("Set constant framerate of %x ms in %x", *constant_rate, get_displayed_name(source_animation, image_defs));

				post_new_frames(id, new_frames, description);
			};

			on_current_or_ticked(set_constant_rate);
		}
	}

	return false;
}
