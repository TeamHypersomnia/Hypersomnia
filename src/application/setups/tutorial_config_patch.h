#pragma once
#include "augs/log.h"
#include "augs/readwrite/json_readwrite.h"
#include "application/config_json_table.h"

inline const rapidjson::Document& get_tutorial_config_patch() {
	static const auto patch = []() {
		const auto path = augs::path_type("content/menu/tutorial_config.json");

		try {
			auto loaded_patch = augs::json_document_from(path);
			auto scratch_config_proving_the_patch_applies = config_json_table();
			augs::read_json(loaded_patch, scratch_config_proving_the_patch_applies);
			return loaded_patch;
		}
		catch (const augs::json_deserialization_error& err) {
			LOG("Failed to load %x:\n%x\nThe tutorial will apply no patch to config.", path, err.what());
		}
		catch (const augs::file_open_error& err) {
			LOG("Failed to load %x:\n%x\nThe tutorial will apply no patch to config.", path, err.what());
		}

		auto no_patch = rapidjson::Document();
		no_patch.SetObject();
		return no_patch;
	}();

	return patch;
}

inline void apply_tutorial_config_patch_with_volumes_as_multipliers(config_json_table& config) {
	const auto players_own_volume = config.audio_volume;

	config.audio_volume.master = 1.f;
	config.audio_volume.sound_effects = 1.f;
	config.audio_volume.music = 1.f;

	augs::read_json(::get_tutorial_config_patch(), config);

	config.audio_volume.master *= players_own_volume.master;
	config.audio_volume.sound_effects *= players_own_volume.sound_effects;
	config.audio_volume.music *= players_own_volume.music;
}
