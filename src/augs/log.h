#pragma once
#include <cstddef>
#include <vector>
#include <cstring>

#include "augs/log_direct.h"
#include "augs/string/typesafe_sprintf.h"
#include "augs/build_settings/setting_enable_debug_log.h"
#include "augs/build_settings/compiler_defines.h"

struct log_entry {
	std::string text;
};

class program_log {
	static program_log global_instance;
	unsigned max_all_entries;

	void push_entry(const log_entry&);
	friend void LOG_NOFORMAT(const std::string& f);

public:
	static auto& get_current() {
		return global_instance;
	}

	program_log(const unsigned max_all_entries);

	std::vector<log_entry> all_entries;
	std::size_t init_logs_count = 0;

	void mark_last_init_log();
	std::size_t get_init_logs_count() const;
	std::size_t get_init_logs_count_nomutex() const;

	std::string get_complete() const;
};

template <class... A>
FORCE_NOINLINE void LOG(const std::string& f, A&&... a) {
	LOG_NOFORMAT(typesafe_sprintf(f, std::forward<A>(a)...));
}

#define LOG_NVPS(...) { \
std::ostringstream sss;\
write_nvps(sss, #__VA_ARGS__, __VA_ARGS__);\
LOG("(%x)", sss.str());\
}

template <typename H1> 
std::ostream& write_nvps(
	std::ostream& out, 
	const char* const label, 
	H1&& value
) {
	return pretty_print(out << label << "=", value);
}

template<typename H1, typename... T> 
std::ostream& write_nvps(
	std::ostream& out, 
	const char* const label, 
	H1&& value, 
	T&&... rest
) {
	const char* const pcomma = strchr(label, ',');

	return write_nvps(
		pretty_print(out.write(label, pcomma - label) << "=", std::forward<H1>(value)) << ',',
		pcomma + 1,
		std::forward<T>(rest)...
	);
}

#if ENABLE_DEBUG_LOG
#define DEBUG_LOG LOG
#else
#define DEBUG_LOG(...)
#endif

std::string& LOG_THREAD_PREFFIX();
