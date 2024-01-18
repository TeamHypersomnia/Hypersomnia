#pragma once
#include "application/network/requested_client_settings.h"
#include "game/modes/mode_player_id.h"

struct synced_meta_update {
	mode_player_id subject_id;
	public_client_settings new_settings;
};
