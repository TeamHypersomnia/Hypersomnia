#include "3rdparty/yojimbo/yojimbo.h"
#undef write_bytes
#undef read_bytes

#include "augs/network/network_types.h"
#include "augs/log.h"
#include <cstdio>
#include <stdarg.h> 

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

LOG(Buffer);  // finaly print the text
return 0;
}

namespace augs {
	namespace network {
		void enable_detailed_logs(const bool flag) {
			yojimbo_log_level(flag ? YOJIMBO_LOG_LEVEL_DEBUG : YOJIMBO_LOG_LEVEL_INFO);
		}

		bool init() {
			LOG("Initializing the network library.");
			const bool result = InitializeYojimbo();

			yojimbo_log_level(YOJIMBO_LOG_LEVEL_INFO);
			yojimbo_set_printf_function(custom_print);
			return result;
		}

		bool deinit() {
			LOG("Shutting down the network library.");
			ShutdownYojimbo();
			return true;
		}
	}
}