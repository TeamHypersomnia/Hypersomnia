#pragma once
#include "error.h"
#define UNICODE
#include <Windows.h>
#undef min
#undef max
#include <gl/glew.h>
#include "../misc/stream.h"
#include <cassert>
namespace augs {
	using namespace error_logging;
	log_file global_log;

	module errors(module::last_error, module::last_error, &global_log);
	module glew_errors(module::glew_last_error, module::glew_last_error, &global_log);
	
	namespace window {
		module errors(module::last_error, module::last_error, &global_log);
	}
	
	namespace texture_baker {
		module errors(module::gl_last_error, module::gl_last_error, &global_log);
	}

	namespace network {
		module errors(module::wsa_last_error, module::wsa_last_error, &global_log);
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
			wcscpy(msgbuf, misc::wstr(glGetError()).c_str());
			//const GLubyte* glstr = gluErrorString(glGetError());
			//MultiByteToWideChar(CP_UTF8, 0, (const char*)glstr, -1, msgbuf, 1000);
		}
		
		void module::glew_last_error(wchar_t* msgbuf) {
			//wcscpy(msgbuf, misc::wstr(glewGetErrorString(glew_last_errorcode)).c_str());
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

		int module::_err(int expression, long lin, const char* fil,  const char* fun) {
			assert(parent && "log_file* parent not specified! use set_parent before enabling logging");
			auto& logs = parent->logs;
			if((!expression || logsucc) && logs.is_open() && enabled) {  
				wchar_t er[1000]; 

				parent->enter(); 

				if(expression && logsucc) 
					logs << "OPERATION HAS COMPLETED SUCCESSFULLY! This message's been generated only because of \"log_succesfull\" flag set to true." << std::endl;
				if(error_func && errorid_func) {
					error_func(er);  
					logs << "line: " << lin << "\nfile: " << fil << "\nfunc: " << fun << "\nerror " << errorid_func() << ": " <<  er << std::endl; 
				}
				else
					logs << "line: " << lin << "\nfile: " << fil << "\nfunc: " << fun; 
				logs << std::endl;

				parent->leave(); 
			} 
			return expression; 
		}

		int module::_errs(int expression, long lin, const char* fil,  const char* fun, const char* strr) {
			wchar_t er[1000];
			MultiByteToWideChar(CP_UTF8, 0, strr, -1, er, 1000);
			return _errs(expression, lin, fil, fun, er);
		}

		int module::_errs(int expression, long lin, const char* fil,  const char* fun, const wchar_t* strr) {
			assert(parent && "log_file* parent not specified! use set_parent before enabling logging");
			auto& logs = parent->logs;
			if((!expression || logsucc) && logs.is_open() && enabled) {  
				wchar_t er[1000]; 
				parent->enter(); 

				if(expression && logsucc) 
					logs << "OPERATION HAS COMPLETED SUCCESSFULLY! This message's been generated only because of \"log_succesfull\" flag set to true." << std::endl;
				if(error_func && errorid_func) {
					error_func(er); 
					logs << "line: " << lin << "\nfile: " << fil << "\nfunc: " << fun << "\nerror " << errorid_func() << ": " <<  er << std::endl << "message: " << strr << std::endl; 
				}
				else 
					logs << "line: " << lin << "\nfile: " << fil << "\nfunc: " << fun << "\nmessage: " << strr << std::endl; 
				logs << std::endl;
				parent->leave(); 
			}
			return expression;
		}

		module::module(void (*errf)(wchar_t*), int(*errid)(), log_file* parent) : parent(parent), error_func(errf), errorid_func(errid), enabled(true), logsucc(false) { }

		void module::enable(bool f) {
			enabled = f;
		}

		void module::log_successful(bool f) {
			logsucc = f;
		}

		log_file::log_file() { dummy = malloc(sizeof(CRITICAL_SECTION)); InitializeCriticalSection((LPCRITICAL_SECTION)dummy); }

		void log_file::open(const wchar_t* fname) {
			logs.open(fname);
		}

		void log_file::close() {
			if(dummy) {
				DeleteCriticalSection((LPCRITICAL_SECTION)dummy);
				free(dummy);
				dummy = 0;
			}
			if(logs.is_open()) logs.close();
		}

		log_file::~log_file() {
			close();
		}

		void log_file::enter() {
			EnterCriticalSection((LPCRITICAL_SECTION)dummy);
		}
		void log_file::leave() {
			LeaveCriticalSection((LPCRITICAL_SECTION)dummy);
		}
	}
}
