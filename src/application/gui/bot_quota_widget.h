#pragma once
#include <cstdint>
#include <algorithm>
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "game/modes/bot_quota.h"

namespace augs::imgui::detail {
	inline void clamp_per_team(int& allies, int& enemies, const int max_total) {
		allies = std::max(allies, 0);
		enemies = std::max(enemies, 0);

		const int total = allies + enemies;

		if (total > max_total) {
			const int dt = total - max_total;

			if (allies < enemies) {
				enemies -= dt;
			}
			else {
				allies -= dt;
			}
		}
	}
}

/*
	Renders a radio that maps to the bot_quota vector size:
	- 0 elements: map default
	- 1 element:  total
	- 2 elements: per team
*/
inline bool edit_bot_quota_widget(bot_quota& q, const int max_per_team = 20) {
	using namespace augs::imgui;

	enum class mode { MAP_DEFAULT, TOTAL, PER_TEAM };

	const auto current_mode =
		q.empty()      ? mode::MAP_DEFAULT :
		q.size() == 1  ? mode::TOTAL :
		mode::PER_TEAM
	;

	bool changed = false;

	auto set_mode = [&](const mode m) {
		if (m == current_mode) {
			return;
		}

		const auto first = q.empty() ? uint8_t(0) : q[0];
		const auto second = q.size() < 2 ? uint8_t(0) : q[1];

		q.clear();

		if (m == mode::TOTAL) {
			q.emplace_back(first == 0 ? uint8_t(7) : first);
		}
		else if (m == mode::PER_TEAM) {
			q.emplace_back(first == 0 ? uint8_t(2) : first);
			q.emplace_back(second == 0 ? uint8_t(5) : second);
		}

		changed = true;
	};

	if (ImGui::RadioButton("Map default##botquota", current_mode == mode::MAP_DEFAULT)) {
		set_mode(mode::MAP_DEFAULT);
	}

	ImGui::SameLine();

	if (ImGui::RadioButton("Total##botquota", current_mode == mode::TOTAL)) {
		set_mode(mode::TOTAL);
	}

	ImGui::SameLine();

	if (ImGui::RadioButton("Per team##botquota", current_mode == mode::PER_TEAM)) {
		set_mode(mode::PER_TEAM);
	}

	auto ind = scoped_indent();

	if (current_mode == mode::TOTAL && !q.empty()) {
		int total = q[0];

		if (slider("Total bots##botquota", total, 0, max_per_team * 2)) {
			changed = true;
		}

		q[0] = uint8_t(std::max(total, 0));
	}
	else if (current_mode == mode::PER_TEAM && q.size() == 2) {
		int allies = q[0];
		int enemies = q[1];

		const bool a_changed = slider("Num allies##botquota", allies, 0, max_per_team);
		const bool b_changed = slider("Num enemies##botquota", enemies, 0, max_per_team);

		augs::imgui::detail::clamp_per_team(allies, enemies, max_per_team * 2);

		q[0] = uint8_t(allies);
		q[1] = uint8_t(enemies);

		if (a_changed || b_changed) {
			changed = true;
		}
	}

	return changed;
}
