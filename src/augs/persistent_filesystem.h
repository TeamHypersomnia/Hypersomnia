#pragma once
#include "augs/misc/scope_guard.h"

void persistent_filesystem_hold();
void persistent_filesystem_flush();
void sync_persistent_filesystem();

struct hold_persistent_filesystem_raii {
	hold_persistent_filesystem_raii() {
		persistent_filesystem_hold();
	}

	~hold_persistent_filesystem_raii() {
		persistent_filesystem_flush();
	}

	hold_persistent_filesystem_raii(const hold_persistent_filesystem_raii&) = delete;
	hold_persistent_filesystem_raii(hold_persistent_filesystem_raii&&) = delete;

	hold_persistent_filesystem_raii& operator=(const hold_persistent_filesystem_raii&) = delete;
	hold_persistent_filesystem_raii& operator=(hold_persistent_filesystem_raii&&) = delete;
};
