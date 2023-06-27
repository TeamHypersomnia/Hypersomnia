#include <csignal>
#include <functional>

#include "augs/log.h"
#include "augs/ensure.h"
#include "augs/ensure_rel.h"
#include "augs/filesystem/file.h"
#include "augs/window_framework/window.h"
#include "augs/window_framework/shell.h"
#include "augs/build_settings/compiler_defines.h"
#include "augs/log_path_getters.h"

extern std::function<void()> ensure_handler;
//void (*ensure_handler)() = nullptr;

void save_log_and_terminate() {
	if (ensure_handler) {
		ensure_handler();
	}

	if (augs::window::current_exists()) {
		augs::window::get_current().set_cursor_clipping(false);
		augs::window::get_current().set_cursor_visible(true);
	}
	
	LOG("\nIf the game crashes repeatedly, consider deleting the \"cache\" folder.");

	const auto logs = program_log::get_current().get_complete();
	const auto failure_log_path = augs::path_type(get_ensure_failed_path());

	mark_as_controlled_crash();
	augs::save_as_text(failure_log_path, logs);

	augs::open_text_editor(failure_log_path.string());

#if !IS_PRODUCTION_BUILD
	/* If we're not in production, trigger a debugger break so that we know the trace */
#if PLATFORM_WINDOWS
	__debugbreak();
#else
	/* On linux we can just script it to always break here */
#endif
#endif

	/* 
		Should force a core dump on Linux platforms, 
		and will also act as a breakpoint for gdb
	*/

	std::abort();	
}

template <class T>
void log_ensure_rel(int type, const char* left_name, const char* right_name, const T& left, const T& right, const char* file, int line) {
	const char* preff = "";

	switch(type) {
		case 0:
			preff = "ensure_eq";
			break;
		case 1:
			preff = "ensure_less";
			break;
		case 2:
			preff = "ensure_leq";
			break;
		case 3:
			preff = "ensure_greater";
			break;
		case 4:
			preff = "ensure_geq";
			break;

		default: break;
	}

    LOG("%x(%x, %x) failed with expansion:\n%x < %x\nfile: %x\nline: %x", preff, left_name, right_name, left, right, file, line);
	save_log_and_terminate();
}

void log_ensure(const char* expr, const char* file, const int line)
{
	LOG("ensure(%x) failed\nfile: %x\nline: %x", expr, file, line);
	save_log_and_terminate();
}

using cvptr = const void*;

template void log_ensure_rel<int>(int, const char*, const char*, const int&, const int&, const char*, int);
template void log_ensure_rel<short>(int, const char*, const char*, const short&, const short&, const char*, int);
template void log_ensure_rel<unsigned short>(int, const char*, const char*, const unsigned short&, const unsigned short&, const char*, int);
template void log_ensure_rel<std::size_t>(int, const char*, const char*, const std::size_t&, const std::size_t&, const char*, int);
template void log_ensure_rel<unsigned>(int, const char*, const char*, const unsigned&, const unsigned&, const char*, int);
template void log_ensure_rel<const void>(int, const char*, const char*, cvptr, cvptr, const char*, int);
template void log_ensure_rel<float>(int, const char*, const char*, const float&, const float&, const char*, int);

#include "application/network/resolve_result_type.h"
#include "game/detail/inventory/wielding_result_type.h"
#include "game/enums/slot_function.h"
#include "view/client_arena_type.h"

template void log_ensure_rel<resolve_result_type>(int, const char*, const char*, const resolve_result_type&, const resolve_result_type&, const char*, int);
template void log_ensure_rel<wielding_result_type>(int, const char*, const char*, const wielding_result_type&, const wielding_result_type&, const char*, int);
template void log_ensure_rel<slot_function>(int, const char*, const char*, const slot_function&, const slot_function&, const char*, int);
template void log_ensure_rel<client_arena_type>(int, const char*, const char*, const client_arena_type&, const client_arena_type&, const char*, int);
