#pragma once
#include <string>
#include <sstream>

namespace augmentations {
	namespace util {

		/* number to string conversion */

		template <class T>
		std::string str(T val) {
			std::ostringstream ss;
			ss << val;
			return ss.str();
		}
		
		/* number to wide string conversion */

		template <class T>
		std::wstring wstr(T val) {
			std::wostringstream ss;
			ss << val;
			return ss.str();
		}
		
		/* wide string to number conversion */

		template <class T>
		T wnum(std::wstring& s) {
			std::wistringstream ss(s);
			T v;
			ss >> v;
			return v;
		}
		
		/* string to number conversion */

		template <class T>
		T num(std::string& s) {
			std::istringstream ss(s);
			T v;
			ss >> v;
			return v;
		}
		
		template <class A, class B>
		void bound(std::wstring& s, A minval, B maxval) {
			if(s.empty()) s = wstr(minval);
			else {
				if(wnum<A>(s) < minval) s = wstr(minval);
				else if(wnum<B>(s) > maxval) s = wstr(maxval);
			}
		}
	}
}