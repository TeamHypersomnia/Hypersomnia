#pragma once
#include "application/network/requested_client_settings.h"
#include "game/modes/mode_player_id.h"

struct public_settings_update {
	mode_player_id subject_id;
	public_client_settings new_settings;
};
