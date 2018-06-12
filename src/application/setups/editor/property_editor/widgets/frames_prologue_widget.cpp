#include "augs/filesystem/file.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "application/setups/editor/property_editor/widgets/frames_prologue_widget.h"
#include "application/setups/editor/detail/field_address.h"

#include "application/setups/editor/detail/get_id_or_import.h"
#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/byte_readwrite.h"

void cut_number_at_end(std::string& s) {
	if (const auto it = s.find_last_not_of("0123456789"); it != std::string::npos) {
		const auto len = s.size() - 1 - it;
		s.erase(s.end() - len, s.end());
	}
}

std::optional<unsigned long> get_number_at_end(const std::string& s) {
	try {
		return std::stoul(s.substr(s.find_last_not_of("0123456789") + 1));
	}
	catch (...) {

	}

	return std::nullopt;
}

bool frames_prologue_widget::handle_prologue(const std::string&, plain_animation_frames_type&) const {
	using namespace augs::imgui;

	using change_prop_command = change_asset_property_command<animation_id_type>;

	const auto& logicals = cmd_in.get_logical_assets();
	const auto& defs = cmd_in.get_viewable_defs();
	const auto& image_defs = defs.image_definitions;

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
		cmd.property_id.field = MACRO_MAKE_FIELD_ADDRESS(plain_animation, frames);
		cmd.value_after_change = augs::to_bytes(new_frames);
		cmd.built_description = std::forward<decltype(description)>(description);

		post_editor_command(cmd_in, std::move(cmd));
	};

	if (ImGui::Button("From file sequence")) {
		auto import_frames = [&](const auto& id) {
			const auto& source_animation = *logicals.find(id);
			const auto& frames = source_animation.frames;

			const auto& first_image_def = image_defs[frames[0].image_id];
			const auto& first_source_path = first_image_def.get_source_path();
			const auto first_path_no_ext = augs::path_type(first_source_path.path).replace_extension("");

			using frames_type = remove_cref<decltype(frames)>;
			using frame_type = typename frames_type::value_type;

			frames_type new_frames;
			new_frames.push_back(frames[0]);

			if (const auto num = get_number_at_end(first_path_no_ext)) {
				auto first_path_no_num = first_path_no_ext.string();
				cut_number_at_end(first_path_no_num);

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

			{
				auto second_half = new_frames;
				reverse_range(second_half);
				second_half.erase(second_half.begin());

				for (auto& s : second_half) {
					if (container_full(new_frames)) {
						break;
					}

					new_frames.push_back(s);
				}
			}

			auto description = typesafe_sprintf("Ping-ponged frames in %x", get_displayed_name(source_animation, image_defs));
			post_new_frames(id, new_frames, description);
		};

		on_current_or_ticked(reverse_frames);
	}

	return false;
}
