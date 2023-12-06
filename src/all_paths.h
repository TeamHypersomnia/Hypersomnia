#pragma once
#include "augs/filesystem/path.h"

/* 
	Hardcoded, relative paths for directories that the executable might use.
*/

/* 
	Can be overridden with --documents-dir.
	If not specified, it will be by default:

	- Windows: Documents/My Games/Hypersomnia/
	- Linux: ~/.config/Hypersomnia/
	- MacOS: Just the CWD temporarily.
*/

extern augs::path_type DOCUMENTS_DIR;

/* 
	From where will the game pull its official resources.
*/

#define OFFICIAL_CONTENT_FOLDER_NAME "content"

/*
	Some essentials lua scripts, ssh-keygen, and internal data files needed by the executable.
*/

#define DETAIL_FOLDER_NAME "detail"

/*
	Where the game will save logs.
	Deleting this folder will have no effect on the game.
*/

#define LOGS_FOLDER_NAME "logs"

/*
	Where the game will save neon maps and other regenerable content.
	Deleting this folder will have no effect on the game, except that some resources might take time to generate again.
*/

#define CACHE_FOLDER_NAME "cache"

/*
	Where the game will save user arenas, all downloaded arenas, gui layouts, user config etc.
	Deleting this folder might possibly result in data loss, e.g. the untitled works and some user settings.

	For Steam clients, this folder will be named after the Steam ID.
*/

#define NONSTEAM_USER_FOLDER_NAME "user"


/*
	Expected to be always in the CWD.
*/

#define OFFICIAL_CONTENT_DIR augs::path_type(OFFICIAL_CONTENT_FOLDER_NAME)
#define DETAIL_DIR augs::path_type(DETAIL_FOLDER_NAME)

/*
	App-data specific.
*/

#define LOGS_DIR (DOCUMENTS_DIR / LOGS_FOLDER_NAME)
#define CACHE_DIR (DOCUMENTS_DIR / CACHE_FOLDER_NAME)

/*
	DOCUMENTS_DIR with appended Steam ID or NONSTEAM_USER_FOLDER_NAME ("user")
	Must be resolved at runtime.
*/

extern augs::path_type USER_DIR;

/*
	Derived subdirectories.
*/

#define USER_DOWNLOADS_DIR 		(USER_DIR / "downloads")
#define OFFICIAL_ARENAS_DIR  	(OFFICIAL_CONTENT_DIR / "arenas")
#define DOWNLOADED_ARENAS_DIR 	(USER_DOWNLOADS_DIR / "arenas")
#define DEMOS_DIR (USER_DIR / "demos")
#define EDITOR_PROJECTS_DIR (USER_DIR / "projects")

