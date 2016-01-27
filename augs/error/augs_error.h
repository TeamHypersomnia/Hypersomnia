#pragma once
#include <fstream>
#include "../options.h"

namespace augs {
	namespace error_logging {
		class log_file {
			void* dummy, enter(), leave();
			friend class module;
		public:
			std::wofstream logs;

			log_file();
			~log_file();
			void open(const wchar_t* fname);
			void close();
		};
	}

	extern error_logging::log_file global_log;

	namespace error_logging {
		extern unsigned glew_last_errorcode;

		class module {
			static unsigned long lasterr();
		public:
			static void wsa_last_error(wchar_t*);
			static void     last_error(wchar_t*);
			static void  gl_last_error(wchar_t*);
			static void  glew_last_error(wchar_t*);

			static int  wsa_last_error();
			static int      last_error();
			static int   gl_last_error();
			static int   glew_last_error();

			int _err(int expression, long lin, const char* fil,  const char* fun);
			int _errs(int expression, long lin, const char* fil,  const char* fun, const char* strr);
			int _errs(int expression, long lin, const char* fil,  const char* fun, const wchar_t* strr);

			log_file* parent;

			module(void (*error_func)(wchar_t*), int(*errorid_func)() = last_error, log_file* parent = &global_log);

			void (*error_func)(wchar_t*);
			int (*errorid_func)();
			bool enabled, logsucc;
			void enable(bool = true);
			void log_successful(bool = false);
			void set_parent(log_file*);

			template<typename T>
			std::wofstream& operator<<(const T& t) {
				return parent.logs << t;
			}
		};
	}

	extern error_logging::module errors;
	extern error_logging::module glew_errors;

	namespace window {
		extern error_logging::module errors;
	}

	namespace texture_baker {
		extern error_logging::module errors;
	}

	namespace network {
		extern error_logging::module errors;
	}
}

#define err(expression) errors._err(int(expression), __LINE__, __FILE__, __FUNCTION__)
#define errl(expression, errors) errors._err(int(expression), __LINE__, __FILE__, __FUNCTION__)
#define errs(expression, str) errors._errs(int(expression), __LINE__, __FILE__, __FUNCTION__, str)
#define errsl(expression, errors, str) errors._errs(int(expression), __LINE__, __FILE__, __FUNCTION__, str)
#define errf(expression, retflag) retflag = retflag ? errors._err(int(expression), __LINE__, __FILE__, __FUNCTION__) : 0;
#define errsf(expression, str, retflag) retflag = retflag ? errors._errs(int(expression), __LINE__, __FILE__, __FUNCTION__, str) : 0;