#pragma once

/*
	A web platform's own settings over the game's - CrazyGames' muteAudio and disableChat, which common.js keeps
	up to date as they change. Elsewhere both are false. Defined in work.cpp.
*/

bool platform_muted_audio();
bool platform_disables_chat();
