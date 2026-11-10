#pragma once
#include <optional>
#include "augs/string/typesafe_sprintf.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/maybe_different.h"
#include "application/setups/editor/playtest_recording/editor_playtest_replay.h"
#include "application/setups/editor/playtest_recording/playtest_replay_speed_ranges.h"

inline std::string describe_slow_motion_edge(const std::optional<double> secs) {
	return secs.has_value() ? typesafe_sprintf("%2f s", *secs) : std::string("?");
}

inline void perform_slow_motion_controls(editor_playtest_replay& shown) {
	using namespace augs::imgui;

	auto& draft = shown.slow_motion_draft;
	auto& ranges = shown.speed_ranges.ranges;
	const auto now_secs = shown.get_current_secs();

	text("Slow motion");
	ImGui::SameLine();

	if (ImGui::Button("From here")) {
		draft.from_secs = now_secs;
	}

	ImGui::SameLine();

	if (ImGui::Button("To here")) {
		draft.to_secs = now_secs;
	}

	ImGui::SameLine();
	ImGui::SetNextItemWidth(110);
	slider("##slow_motion_speed", draft.speed, 0.1f, 1.0f, "%.2fx");

	const bool draft_complete =
		draft.from_secs.has_value()
		&& draft.to_secs.has_value()
		&& *draft.to_secs > *draft.from_secs
	;

	text_disabled(typesafe_sprintf("%x - %x", ::describe_slow_motion_edge(draft.from_secs), ::describe_slow_motion_edge(draft.to_secs)));
	ImGui::SameLine();

	{
		auto scope = maybe_disabled_cols({}, !draft_complete);

		if (ImGui::Button("Add range")) {
			::add_replay_speed_range(shown.speed_ranges, { *draft.from_secs, *draft.to_secs, static_cast<double>(draft.speed) });
			::save_replay_speed_ranges(shown.recording.paths, shown.speed_ranges);
			draft.from_secs = std::nullopt;
			draft.to_secs = std::nullopt;
		}
	}

	if (ImGui::IsItemHovered()) {
		text_tooltip("Plays this part of the recording slower - also when rendered from the command line,\nwhere it gets 2+ frames per step, so it stays smooth. The sounds slow down and drop in pitch.\nSaved next to the recording in speed_ranges.json.");
	}

	std::optional<std::size_t> removed;

	for (std::size_t i = 0; i < ranges.size(); ++i) {
		const auto& range = ranges[i];
		auto id = scoped_id(static_cast<int>(i));

		text(typesafe_sprintf("%2f s - %2f s at %2fx", range.from_secs, range.to_secs, range.speed));
		ImGui::SameLine();

		if (ImGui::Button("Remove")) {
			removed = i;
		}
	}

	if (removed.has_value()) {
		ranges.erase(ranges.begin() + *removed);
		::save_replay_speed_ranges(shown.recording.paths, shown.speed_ranges);
	}
}
