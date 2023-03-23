#pragma once

inline void setup_project_defaults(
	editor_arena_settings& defaults,
	const editor_official_resource_map& o
) {
	defaults.warmup_theme.id = o[test_sound_decorations::ARABESQUE];
}

template <class M>
inline void setup_default_playtesting_mode(
	editor_playtesting_settings& defaults,
	const M& modes
) {
	/*
		First look for one named specifically playtesting, then any playtesting mode, then any mode.
	*/

	using T = decltype(defaults.mode);

	modes.for_each_id_and_object([&](const auto id, const auto& object) {
		if (object.type.template is<editor_playtesting_mode>() && object.unique_name == "playtesting") {
			if (!defaults.mode.is_set()) {
				defaults.mode = T::from_raw(id, false);
			}
		}
	});

	modes.for_each_id_and_object([&](const auto id, const auto& object) {
		if (object.type.template is<editor_playtesting_mode>()) {
			if (!defaults.mode.is_set()) {
				defaults.mode = T::from_raw(id, false);
			}
		}
	});

	modes.for_each_id_and_object([&](const auto id, const auto&) {
		if (!defaults.mode.is_set()) {
			defaults.mode = T::from_raw(id, false);
		}
	});
}

template <class M>
void setup_project_defaults(
	editor_playtesting_settings& defaults,
	const M& modes,
	const editor_official_resource_map& o
) {
	::setup_default_playtesting_mode(defaults, modes);
	(void)o;
}
