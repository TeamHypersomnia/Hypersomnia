#include <atomic>
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <exception>

#include "augs/window_framework/crash_report.h"

#if PLATFORM_WINDOWS
#include <Windows.h>
#undef min
#undef max

#include "augs/log.h"
#include "augs/misc/mutex.h"
#include "augs/filesystem/file.h"
#include "augs/window_framework/shell.h"

extern augs::mutex log_mutex;

namespace augs {
	static bool show_crash_dialog = false;
	static std::string crash_log_path;
	static std::atomic_flag crash_being_reported = ATOMIC_FLAG_INIT;

	void show_error_dialog(const std::string& title, const std::string& text) {
		MessageBoxA(nullptr, text.c_str(), title.c_str(), MB_OK | MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
	}

	static bool ask_in_error_dialog(const std::string& title, const std::string& text) {
		return MessageBoxA(nullptr, text.c_str(), title.c_str(), MB_YESNO | MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND) == IDYES;
	}

	static std::string logs_ending_with(const std::string& crash_reason) {
		const auto attempts_to_lock_the_log = 100;
		const auto ms_between_attempts = 10;

		for (int attempt = 0; attempt < attempts_to_lock_the_log; ++attempt) {
			if (log_mutex.try_lock()) {
				auto& log = program_log::get_current();
				log.push_entry({ crash_reason });
				const auto logs = log.get_complete_nomutex();
				log_mutex.unlock();

				return logs;
			}

			Sleep(ms_between_attempts);
		}

		return "The log was locked by the crashed thread.\n" + crash_reason + "\n";
	}

	static bool save_crash_log(const std::string& crash_reason) {
		if (crash_log_path.empty()) {
			return false;
		}

		try {
			save_as_text(crash_log_path, logs_ending_with(crash_reason));
			return true;
		}
		catch (...) {
			return false;
		}
	}

	static void report_crash_and_exit(const std::string& crash_reason) {
		if (crash_being_reported.test_and_set()) {
			Sleep(INFINITE);
		}

		ClipCursor(nullptr);

		const bool log_saved = save_crash_log(crash_reason);

		if (show_crash_dialog) {
			if (log_saved) {
				const auto text = "Hypersomnia has crashed.\n\n" + crash_reason + "\n\nThe log was saved to:\n" + crash_log_path + "\n\nOpen it?";

				if (ask_in_error_dialog("Hypersomnia", text)) {
					open_text_editor(crash_log_path);
				}
			}
			else {
				show_error_dialog("Hypersomnia", "Hypersomnia has crashed.\n\n" + crash_reason + "\n\nThe log could not be saved.");
			}
		}

		TerminateProcess(GetCurrentProcess(), EXIT_FAILURE);
	}

	static std::string describe_exception_code(const DWORD code) {
		switch (code) {
			case EXCEPTION_ACCESS_VIOLATION: return "Access violation";
			case EXCEPTION_STACK_OVERFLOW: return "Stack overflow";
			case EXCEPTION_ILLEGAL_INSTRUCTION: return "Illegal instruction";
			case EXCEPTION_INT_DIVIDE_BY_ZERO: return "Integer division by zero";
			default: return "Exception";
		}
	}

	static std::string describe_module_at(const void* const address) {
		auto module = HMODULE();
		const auto flags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;

		if (GetModuleHandleExW(flags, static_cast<LPCWSTR>(address), &module)) {
			wchar_t module_path[MAX_PATH];

			if (GetModuleFileNameW(module, module_path, MAX_PATH) > 0) {
				const auto module_name = path_type(module_path).filename();
				const auto offset = reinterpret_cast<std::uintptr_t>(address) - reinterpret_cast<std::uintptr_t>(module);

				return typesafe_sprintf("%x+0x%h", string_windows_friendly(module_name), offset);
			}
		}

		return typesafe_sprintf("0x%h", reinterpret_cast<std::uintptr_t>(address));
	}

	static LONG WINAPI report_unhandled_exception(EXCEPTION_POINTERS* const exception) {
		const auto& record = *exception->ExceptionRecord;
		const auto code = record.ExceptionCode;

		const auto accessed_address = [&]() -> std::string {
			if (code == EXCEPTION_ACCESS_VIOLATION && record.NumberParameters >= 2) {
				const auto access = record.ExceptionInformation[0] == 1 ? "writing" : "reading";
				return typesafe_sprintf(" %x 0x%h", access, record.ExceptionInformation[1]);
			}

			return "";
		}();

		report_crash_and_exit(typesafe_sprintf(
			"%x%x (code 0x%h) in %x",
			describe_exception_code(code),
			accessed_address,
			code,
			describe_module_at(record.ExceptionAddress)
		));

		return EXCEPTION_EXECUTE_HANDLER;
	}

	static void report_terminate() {
		const auto reason = []() -> std::string {
			if (const auto current = std::current_exception(); current != nullptr) {
				try {
					std::rethrow_exception(current);
				}
				catch (const std::exception& err) {
					return std::string("Uncaught exception: ") + err.what();
				}
				catch (...) {
					return "Uncaught exception of an unknown type.";
				}
			}

			return "std::terminate was called.";
		}();

		report_crash_and_exit(reason);
		std::abort();
	}

	static void report_abort(int) {
		report_crash_and_exit("abort() was called.");
	}

	static void report_invalid_parameter(const wchar_t*, const wchar_t*, const wchar_t*, unsigned, std::uintptr_t) {
		report_crash_and_exit("Invalid parameter passed to a C runtime function.");
	}

	static void report_pure_virtual_call() {
		report_crash_and_exit("Pure virtual function call.");
	}

	void report_crashes(const bool show_dialog) {
		show_crash_dialog = show_dialog;

		auto stack_kept_for_crash_report = ULONG(64 * 1024);
		SetThreadStackGuarantee(&stack_kept_for_crash_report);

		SetUnhandledExceptionFilter(report_unhandled_exception);
		std::set_terminate(report_terminate);
		std::signal(SIGABRT, report_abort);
		_set_invalid_parameter_handler(report_invalid_parameter);
		_set_purecall_handler(report_pure_virtual_call);
	}

	void set_crash_log_path(const std::string& path) {
		crash_log_path = path;
	}
}
#else
namespace augs {
	void report_crashes(bool) {}
	void set_crash_log_path(const std::string&) {}

	void show_error_dialog(const std::string&, const std::string&) {}
}
#endif
