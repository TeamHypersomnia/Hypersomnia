#pragma once
#include "application/network/requested_client_settings.h"
#include "game/modes/mode_player_id.h"
#include "view/mode_gui/arena/arena_player_meta.h"

struct synced_meta_update {
	mode_player_id subject_id;
	synced_player_meta new_meta;
};
