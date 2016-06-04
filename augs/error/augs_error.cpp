#pragma once
// legacy functionality kept for conformance with old code. Do not use anymore.
#include "augs_error.h"

#if ENABLE_LEGACY_ERROR_REPORTING
#include <Windows.h>
#undef min
#undef max
#include <GL/OpenGL.h>
#include "../stream.h"
#include "log.h"

namespace augs {
	using namespace error_logging;

	module errors = module(module::last_error, module::last_error);
	module glew_errors = module(module::glew_last_error, module::glew_last_error);
	
	namespace window {
		module errors = module(module::last_error, module::last_error);
	}
	
	namespace texture_baker {
		module errors = module(module::gl_last_error, module::gl_last_error);
	}

	namespace network {
		module errors = module(module::wsa_last_error, module::wsa_last_error);
	}

	namespace error_logging {
		unsigned glew_last_errorcode = 0;

		void module::last_error(wchar_t* msgbuf) {
			DWORD dw = GetLastError(); 
			FormatMessage(
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				dw,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				msgbuf,
				1000, NULL );
		}

		void module::wsa_last_error(wchar_t* msgbuf) {
			DWORD dw = WSAGetLastError(); 
			FormatMessage(
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				dw,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				msgbuf,
				1000, NULL );
		}

		void module::gl_last_error(wchar_t* msgbuf) {
			wcscpy(msgbuf, to_wstring(glGetError()).c_str());
		}
		
		void module::glew_last_error(wchar_t* msgbuf) {
			const GLubyte* glstr = glewGetErrorString(glew_last_errorcode);
			MultiByteToWideChar(CP_UTF8, 0, (const char*)glstr, -1, msgbuf, 1000);
		}

		int module::wsa_last_error() {
			return WSAGetLastError();
		}

		int module::last_error() {
			return GetLastError();
		}
		
		int module::gl_last_error() {
			return glGetError();
		}

		int module::glew_last_error() {
			return glew_last_errorcode;
		}

		int module::_err(int expression, long lin, const char* fil, const char* fun) {
			if(!expression && enabled) {  
				wchar_t er[1000]; 

				LOG("line: %x\nfile: %x\nfunc: %x", lin, fil, fun);

				if(error_func && errorid_func) {
					error_func(er);  
					LOG("error %x: %x", errorid_func(), er);
				}
			} 
			return expression; 
		}

		int module::_errs(int expression, long lin, const char* fil,  const char* fun, const char* strr) {
			wchar_t er[1000];
			MultiByteToWideChar(CP_UTF8, 0, strr, -1, er, 1000);
			return _errs(expression, lin, fil, fun, er);
		}

		int module::_errs(int expression, long lin, const char* fil,  const char* fun, const wchar_t* strr) {
			if(!expression && enabled) {
				_err(expression, lin, fil, fun);
				LOG("message: %x", strr);
			}
			return expression;
		}

		module::module(void (*errf)(wchar_t*), int(*errid)()) : error_func(errf), errorid_func(errid) { }
	}
}
#endif
