#pragma once
#if PLATFORM_WEB
#include "augs/misc/mutex.h"

EM_JS(void, call_syncFileSystem, (), {
  Module.sync_idbfs();
});

augs::mutex persistent_filesystem_lk;
int persistent_filesystem_holders = 0;

void persistent_filesystem_sync() {
	{
		augs::scoped_lock lk(persistent_filesystem_lk);

		if (persistent_filesystem_holders > 0) {
			return;
		}
	}

	LOG("persistent_filesystem_sync");

	main_thread_queue::get_instance().execute(
		[]() {
			call_syncFileSystem();
		}
	);
}

void persistent_filesystem_hold() {
	augs::scoped_lock lk(persistent_filesystem_lk);
	++persistent_filesystem_holders;
	LOG("persistent_filesystem_hold: %x", persistent_filesystem_holders);
}

void persistent_filesystem_flush() {
	bool run = false;

	{
		augs::scoped_lock lk(persistent_filesystem_lk);
		--persistent_filesystem_holders;

		LOG("persistent_filesystem_hold: %x", persistent_filesystem_holders);

		if (persistent_filesystem_holders == 0) {
			run = true;
		}
	}

	if (run) {
		persistent_filesystem_sync();
	}
}

#else
void persistent_filesystem_hold() {}
void persistent_filesystem_flush() {}
void persistent_filesystem_sync() {}
#endif
