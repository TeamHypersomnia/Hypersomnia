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
#include "augs/filesystem/file.h"
#include "augs/window_framework/platform_utils.h"

namespace augs {
	constexpr DWORD msvc_cpp_exception_code_v = 0xE06D7363;
	constexpr UINT abort_exit_code_v = 3;
	constexpr DWORD ms_given_to_the_first_crash_report_v = 10000;
	constexpr int attempts_to_lock_the_log_v = 100;
	constexpr DWORD ms_between_log_lock_attempts_v = 10;

	static std::atomic<bool> crash_reports_enabled = false;
	static std::atomic_flag crash_being_reported = ATOMIC_FLAG_INIT;
	static bool open_log_in_notepad_after_crash = false;
	static path_type error_log_path;
	static LPTOP_LEVEL_EXCEPTION_FILTER previous_exception_filter = nullptr;

	static std::string logs_ending_with(const std::string& crash_reason) {
		for (auto attempt = 0; attempt < attempts_to_lock_the_log_v; ++attempt) {
			if (auto logs = program_log::get_current().try_get_complete_with({ crash_reason }); logs.has_value()) {
				return *logs;
			}

			Sleep(ms_between_log_lock_attempts_v);
		}

		return "The log was locked by the crashed thread.\n" + crash_reason + "\n";
	}

	static bool save_error_log(const std::string& crash_reason) {
		if (error_log_path.empty()) {
			return false;
		}

		try {
			save_as_text(error_log_path, logs_ending_with(crash_reason));
			return true;
		}
		catch (...) {
			return false;
		}
	}

	static bool open_in_notepad(const path_type& path) {
		auto command_line = L"notepad.exe \"" + path.wstring() + L"\"";
		auto startup_info = STARTUPINFOW();
		startup_info.cb = sizeof(startup_info);
		auto process_info = PROCESS_INFORMATION();

		const auto started = CreateProcessW(
			nullptr,
			command_line.data(),
			nullptr,
			nullptr,
			FALSE,
			0,
			nullptr,
			nullptr,
			&startup_info,
			&process_info
		);

		if (!started) {
			return false;
		}

		CloseHandle(process_info.hThread);
		CloseHandle(process_info.hProcess);
		return true;
	}

	static void show_crash_dialog(const std::string& crash_reason, const std::string& log_state) {
		const auto log_path = error_log_path.empty() ? std::wstring() : L"\n" + error_log_path.wstring();
		const auto text = widen("Hypersomnia has crashed.\n\n" + crash_reason + "\n\n" + log_state) + log_path;
		const auto style = MB_OK | MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND;

		MessageBoxW(nullptr, text.c_str(), L"Hypersomnia", style);
	}

	static void report_crash_and_exit(const std::string& crash_reason, const UINT exit_code) {
		if (!crash_reports_enabled) {
			TerminateProcess(GetCurrentProcess(), exit_code);
		}

		if (crash_being_reported.test_and_set()) {
			Sleep(ms_given_to_the_first_crash_report_v);
			TerminateProcess(GetCurrentProcess(), exit_code);
		}

		ClipCursor(nullptr);

		const bool log_saved = save_error_log(crash_reason);

		if (open_log_in_notepad_after_crash) {
			if (!log_saved) {
				show_crash_dialog(crash_reason, "The log could not be saved.");
			}
			else if (!open_in_notepad(error_log_path)) {
				show_crash_dialog(crash_reason, "The log was saved to:");
			}
		}

		TerminateProcess(GetCurrentProcess(), exit_code);
	}

	static std::string describe_exception_code(const DWORD code) {
		switch (code) {
			case EXCEPTION_ACCESS_VIOLATION: return "Access violation";
			case EXCEPTION_STACK_OVERFLOW: return "Stack overflow";
			case EXCEPTION_ILLEGAL_INSTRUCTION: return "Illegal instruction";
			case EXCEPTION_INT_DIVIDE_BY_ZERO: return "Integer division by zero";
			case EXCEPTION_BREAKPOINT: return "Breakpoint";
			case msvc_cpp_exception_code_v: return "Uncaught C++ exception";
			default: return "Exception";
		}
	}

	static std::string describe_access(const ULONG_PTR access_type) {
		switch (access_type) {
			case EXCEPTION_WRITE_FAULT: return "writing";
			case EXCEPTION_EXECUTE_FAULT: return "executing";
			default: return "reading";
		}
	}

	static std::string describe_module_at(const void* const address) {
		auto module = HMODULE();
		const auto flags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;

		if (GetModuleHandleExW(flags, static_cast<LPCWSTR>(address), &module)) {
			if (const auto module_path = get_module_path(module); !module_path.empty()) {
				const auto offset = reinterpret_cast<std::uintptr_t>(address) - reinterpret_cast<std::uintptr_t>(module);
				return typesafe_sprintf("%x+0x%h", string_windows_friendly(module_path.filename()), offset);
			}
		}

		return typesafe_sprintf("0x%h", reinterpret_cast<std::uintptr_t>(address));
	}

	static LONG WINAPI report_unhandled_exception(EXCEPTION_POINTERS* const exception) {
		const auto& record = *exception->ExceptionRecord;
		const auto code = record.ExceptionCode;

		if (code == msvc_cpp_exception_code_v && previous_exception_filter != nullptr) {
			return previous_exception_filter(exception);
		}

		const auto accessed_address = [&]() -> std::string {
			if (code == EXCEPTION_ACCESS_VIOLATION && record.NumberParameters >= 2) {
				const auto& information = record.ExceptionInformation;
				return typesafe_sprintf(" %x 0x%h", describe_access(information[0]), information[1]);
			}

			return "";
		}();

		const auto reason = typesafe_sprintf(
			"%x%x (code 0x%h) in %x",
			describe_exception_code(code),
			accessed_address,
			code,
			describe_module_at(record.ExceptionAddress)
		);

		report_crash_and_exit(reason, code);
		return EXCEPTION_EXECUTE_HANDLER;
	}

	static std::string describe_abort() {
		if (const auto current = std::current_exception(); current != nullptr) {
			try {
				std::rethrow_exception(current);
			}
			catch (const std::exception& err) {
				return std::string("Exception: ") + err.what();
			}
			catch (...) {
				return "Exception of an unknown type.";
			}
		}

		return "abort() was called.";
	}

	static void report_abort(const int signal_number) {
		std::signal(signal_number, report_abort);
		report_crash_and_exit(describe_abort(), abort_exit_code_v);
	}

	static void report_invalid_parameter(const wchar_t*, const wchar_t*, const wchar_t*, unsigned, std::uintptr_t) {
		report_crash_and_exit("Invalid parameter passed to a C runtime function.", abort_exit_code_v);
	}

	static void report_pure_virtual_call() {
		report_crash_and_exit("Pure virtual function call.", abort_exit_code_v);
	}

	void keep_stack_for_crash_report() {
		auto stack_kept_for_crash_report = ULONG(64 * 1024);
		SetThreadStackGuarantee(&stack_kept_for_crash_report);
	}

	void report_crashes(const bool open_log_in_notepad) {
		open_log_in_notepad_after_crash = open_log_in_notepad;
		crash_reports_enabled = true;

		keep_stack_for_crash_report();

		previous_exception_filter = SetUnhandledExceptionFilter(report_unhandled_exception);
		std::signal(SIGABRT, report_abort);
		_set_invalid_parameter_handler(report_invalid_parameter);
		_set_purecall_handler(report_pure_virtual_call);
	}

	void set_error_log_path(const path_type& path) {
		error_log_path = std::filesystem::absolute(path);
	}

	void stop_reporting_crashes() {
		crash_reports_enabled = false;
	}
}
#else
namespace augs {
	void report_crashes(bool) {}
	void keep_stack_for_crash_report() {}
	void set_error_log_path(const path_type&) {}
	void stop_reporting_crashes() {}
}
#endif
