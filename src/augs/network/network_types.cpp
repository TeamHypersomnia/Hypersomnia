#include "3rdparty/yojimbo/include/yojimbo.h"
#undef write_bytes
#undef read_bytes

#include "augs/network/network_types.h"
#include "augs/log.h"
#include "augs/ensure.h"
#include <cstdio>
#include <stdarg.h> 

#if BUILD_WEBRTC
#include "rtc/rtc.hpp"
#endif

bool InitializeYojimbo();
void ShutdownYojimbo();

int custom_print(const char* Format,...)
{
// the “…” indicates that this function accepts a variable number of arguments
char Buffer[5000]; // you can define your own buffer’s size
va_list args;
va_start(args,Format);
vsprintf(Buffer,Format,args);
va_end(args);
// now everything is written on “Buffer” , so it is ready for use with “InGamePrint”

LOG_NOFORMAT(Buffer);  // finaly print the text
return 0;
}

#if BUILD_WEBRTC && !PLATFORM_WEB
std::mutex rtc_errors_lk;
bool track_rtc_errors = false;
std::vector<std::string> rtc_errors;

void rtc_log_callback(rtc::LogLevel level, std::string message) {
	LOG_NOFORMAT(message);

	if (track_rtc_errors) {
		if (level <= rtc::LogLevel::Fatal) {
			std::scoped_lock lk(rtc_errors_lk);
			rtc_errors.push_back(message);
		}
	}
}
#endif

namespace augs {
	namespace network {
		void enable_detailed_logs(const bool flag) {
			yojimbo_log_level(flag ? YOJIMBO_LOG_LEVEL_DEBUG : YOJIMBO_LOG_LEVEL_INFO);
		}

		bool init(bool rtc_errors, bool verbose_rtc) {
			track_rtc_errors = rtc_errors;

			LOG("Initializing the network library.");
			const bool result = InitializeYojimbo();

			yojimbo_log_level(YOJIMBO_LOG_LEVEL_INFO);
			yojimbo_set_printf_function(custom_print);
			yojimbo_set_assert_function(log_ensure);

#if BUILD_WEBRTC
#if !PLATFORM_WEB
			if (verbose_rtc) {
				rtc::InitLogger(rtc::LogLevel::Verbose, rtc_log_callback);
			}
			else {
				rtc::InitLogger(rtc::LogLevel::Info, rtc_log_callback);
			}
#endif
			rtc::Preload();
#endif

			return result;
		}

		bool deinit() {
			LOG("Shutting down the network library.");
#if BUILD_WEBRTC
			rtc::Cleanup();
#endif
			ShutdownYojimbo();
			return true;
		}
	}
}