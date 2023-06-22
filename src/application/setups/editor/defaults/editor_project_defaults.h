#pragma once

template <class M>
inline void setup_default_server_mode(
	editor_arena_settings& defaults,
	const M& modes
) {
	/*
		First look for any mode that is not playtesting,
		then any then any mode.
	*/

	using T = decltype(defaults.default_server_mode);
	auto& mode = defaults.default_server_mode;

	modes.for_each_id_and_object([&](const auto id, const auto& object) {
		if (!object.type.template is<editor_quick_test_mode>()) {
			if (!mode.is_set()) {
				mode = T::from_raw(id, false);
			}
		}
	});

	modes.for_each_id_and_object([&](const auto id, const auto&) {
		if (!mode.is_set()) {
			mode = T::from_raw(id, false);
		}
	});
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
		if (object.type.template is<editor_quick_test_mode>() && object.unique_name == "quick_test") {
			if (!defaults.mode.is_set()) {
				defaults.mode = T::from_raw(id, false);
			}
		}
	});

	modes.for_each_id_and_object([&](const auto id, const auto& object) {
		if (object.type.template is<editor_quick_test_mode>()) {
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

template <class M>
inline void setup_project_defaults(
	editor_arena_settings& defaults,
	const M& modes,
	const editor_official_resource_map& o
) {
	::setup_default_server_mode(defaults, modes);
	defaults.warmup_theme.id = o[test_sound_decorations::ARABESQUE];
}
